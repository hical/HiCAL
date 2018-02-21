#include <iostream>
#include <mutex>
#include <thread>
#include <algorithm>
#include <unordered_set>
#include "bmi_para.h"

using namespace std;
BMI_para::BMI_para(Seed _seed,
        Dataset *_documents,
        Dataset *_paragraphs,
        int _num_threads,
        int _judgments_per_iteration,
        int _max_effort,
        int _max_iterations,
        bool _async_mode)
    :BMI(_seed, _documents, _num_threads, _judgments_per_iteration, _max_effort, _max_iterations, _async_mode, false),
    paragraphs(_paragraphs)
{
    perform_iteration();
}

// Inefficient as of now
void BMI_para::remove_from_judgment_list(int doc_idx){
    lock_guard<mutex> lock(judgment_list_mutex);
    vector<int> keys_to_remove;
    for(pair<int, int> id_rank: judgment_queue_by_id){
        string para_id = paragraphs->get_sf_sparse_vector(id_rank.first).doc_id;
        string doc_id = documents->get_sf_sparse_vector(doc_idx).doc_id;
        if(para_id.find(doc_id) == 0){
            keys_to_remove.push_back(id_rank.first);
            judgment_queue_by_rank.erase(id_rank.second);
        }
    }

    for(auto id: keys_to_remove)
        judgment_queue_by_id.erase(id);
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
            string doc_id = documents->get_sf_sparse_vector(training.first).doc_id;
            int missing = 0;
            for(int i = 0; missing<1000; i++){
                string para_offset = to_string(i);
                string para_id = doc_id + "." + para_offset;
                if(paragraphs->get_index(para_id) == paragraphs->NPOS)
                    missing++;
                else
                    finished_judgments_para[paragraphs->get_index(para_id)] = 1, missing = 0;
            }
        }
        training_cache.clear();
    }

    // Training
    auto start = std::chrono::steady_clock::now();

    SfWeightVector w = train();

    auto weights = w.AsFloatVector();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Training finished in "<<duration.count()<<"ms"<<endl;

    // Scoring
    start = std::chrono::steady_clock::now();
    auto results = paragraphs->rescore(weights, num_threads,
                              judgments_per_iteration + (async_mode ? extra_judgment_docs : 100),
                              finished_judgments_para);
    duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Rescored "<<paragraphs->size()<<" documents in "<<duration.count()<<"ms"<<endl;

    return results;
}

std::vector<std::pair<string, float>> BMI_para::get_ranklist(){
    vector<std::pair<string, float>> ret_results;
    unordered_set<string> doc_id_seen;
    auto results = get_ranking_dataset()->rescore(train().AsFloatVector(), num_threads,
                              get_ranking_dataset()->size(), map<int, int>());

    for(auto result: results){
        string para_id = get_ranking_dataset()->get_sf_sparse_vector(result).doc_id;
        string doc_id = para_id.substr(0, para_id.find('.'));
        if(doc_id_seen.find(doc_id) == doc_id_seen.end()){
            ret_results.push_back({doc_id, 0});
            doc_id_seen.insert(doc_id);
        }
    }
    return ret_results;
}

