#include <iostream>
#include <mutex>
#include <thread>
#include <algorithm>
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
    :BMI(_seed, _scorer, _num_threads, _judgments_per_iteration, _max_effort, _max_iterations, _async_mode),
    scorer_para(_scorer_para)
{
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

void BMI_para::record_judgment(string doc_id, int judgment){
    int id = scorer_para->doc_ids_inv_map[doc_id];
    int doc_id_int = scorer->doc_ids_inv_map[doc_id.substr(0, doc_id.length() - 4)];
    add_to_training_cache(doc_id_int, judgment);
    remove_from_judgment_list(id);

    if(!async_mode){
        if(finished_judgments.size() + training_cache.size() >= state.next_iteration_target)
            perform_iteration();
    }else{
        auto t = thread(&BMI_para::perform_iteration_async, this);
        t.detach();
    }
}

vector<int> BMI_para::perform_training_iteration(){
    lock_guard<mutex> lock_training(training_mutex);

    {
        lock_guard<mutex> lock(training_cache_mutex);
        for(pair<int, int> training: training_cache){
            judgments[training.first] = training.second;
            finished_judgments.insert(training.first);
            string doc_id = (*scorer->doc_features)[training.first]->doc_id;
            for(int i = 0; ; i++){
                string para_offset = to_string(i);
                string para_id = doc_id + "." + string(3 - para_offset.size(), '0') + para_offset;
                if(scorer_para->doc_ids_inv_map.find(para_id) == scorer_para->doc_ids_inv_map.end())
                    break;
                else
                    finished_judgments_para.insert(scorer_para->doc_ids_inv_map[para_id]);
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
    scorer_para->rescore_documents(weights, num_threads, judgments_per_iteration+(async_mode?extra_judgment_docs:0), finished_judgments_para, results);
    duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Rescored "<<scorer->doc_features->size()<<" documents in "<<duration.count()<<"ms"<<endl;

    state.weights = weights;

    return results;
}
