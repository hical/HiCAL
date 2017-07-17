#include <iostream>
#include <mutex>
#include <thread>
#include <algorithm>
#include <unordered_set>
#include "bmi_para.h"

using namespace std;
BMI_para::BMI_para(const SfSparseVector &_seed,
        Scorer *_scorer,
        Scorer *_scorer_para,
        int _num_threads,
        int _judgments_per_iteration,
        int _max_effort,
        int _max_iterations,
        bool _async_mode)
    :BMI(_seed, _scorer, _num_threads, _judgments_per_iteration, _max_effort, _max_iterations, _async_mode, false),
    scorer_para(_scorer_para)
{
    perform_iteration();
}

vector<string> BMI_para::get_doc_to_judge(uint32_t count=1){
    while(true){
        {
            lock_guard<mutex> lock(judgment_list_mutex);
            lock_guard<mutex> lock2(training_cache_mutex);
            if(judgment_list.size() > 0){
                vector<string> ret;
                for(int id: judgment_list){
                    if(id == -1 || ret.size() >= count)
                        break;
                    ret.push_back((*scorer_para->doc_features)[id]->doc_id);
                }
                return ret;
            }
            /* for(int id: judgment_list){ */
            /*     // poison value */
            /*     if(id == -1){ */
            /*         return {}; */
            /*     } */
            /*     if(finished_judgments.find(id) == finished_judgments.end() \ */
            /*             && training_cache.find(id) == training_cache.end()) */
            /*         return doc_features[id].doc_id; */
            /* } */
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void BMI_para::remove_from_judgment_list(int doc_idx){
    lock_guard<mutex> lock(judgment_list_mutex);
    judgment_list.erase(std::remove_if(judgment_list.begin(), judgment_list.end(),
            [doc_idx, this](int id) -> bool {
                string para_id = (*this->scorer_para->doc_features)[id]->doc_id;
                string doc_id = (*this->scorer->doc_features)[doc_idx]->doc_id;
                return para_id.find(doc_id) == 0;
            }
    ), judgment_list.end());
}

void BMI_para::record_judgment(string doc_id, int judgment){
    record_judgment_batch({{doc_id.substr(0, doc_id.find(".")), judgment}});
}

vector<int> BMI_para::perform_training_iteration(){
    lock_guard<mutex> lock_training(training_mutex);

    {
        lock_guard<mutex> lock(training_cache_mutex);
        for(pair<int, int> training: training_cache){
            judgments[training.first] = training.second;
            finished_judgments.insert(training.first);
            string doc_id = (*scorer->doc_features)[training.first]->doc_id;
            int missing = 0;
            for(int i = 0; missing<1000; i++){
                string para_offset = to_string(i);
                string para_id = doc_id + "." + para_offset;
                if(scorer_para->doc_ids_inv_map.find(para_id) == scorer_para->doc_ids_inv_map.end())
                    missing++;
                else
                    finished_judgments_para.insert(scorer_para->doc_ids_inv_map[para_id]), missing = 0;
            }
        }
        training_cache.clear();
    }

    // Training
    auto start = std::chrono::steady_clock::now();

    SfWeightVector w(scorer->dimensionality);
    train(w);

    auto weights = w.AsFloatVector();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Training finished in "<<duration.count()<<"ms"<<endl;

    vector<int> results;

    // Scoring
    start = std::chrono::steady_clock::now();
    scorer_para->rescore_documents(weights, num_threads, judgments_per_iteration+(async_mode?extra_judgment_docs:100), finished_judgments_para, results);
    duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Rescored "<<scorer_para->doc_features->size()<<" documents in "<<duration.count()<<"ms"<<endl;

    state.weights = weights;

    return results;
}

std::vector<std::pair<string, float>> BMI_para::get_ranklist(){
    vector<std::pair<string, float>> ret_results;
    unordered_set<string> doc_id_seen;
    auto results = get_ranking_scorer()->rescore_all_documents(state.weights, num_threads);

    for(auto result: results){
        string para_id = (*get_ranking_scorer()->doc_features)[result.first]->doc_id;
        string doc_id = para_id.substr(0, para_id.find('.'));
        if(doc_id_seen.find(doc_id) == doc_id_seen.end()){
            ret_results.push_back({doc_id, result.second});
            doc_id_seen.insert(doc_id);
        }
    }
    return ret_results;
}

