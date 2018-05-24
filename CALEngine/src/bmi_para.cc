#include <iostream>
#include <mutex>
#include <thread>
#include <algorithm>
#include <unordered_set>
#include "bmi_para.h"
#include "utils/utils.h"

using namespace std;
BMI_para::BMI_para(Seed _seed,
        Dataset *_documents,
        Dataset *_paragraphs,
        int _num_threads,
        int _judgments_per_iteration,
        bool _async_mode,
        int _training_iterations)
    :BMI(_seed, _documents, _num_threads, _judgments_per_iteration, _async_mode, _training_iterations, false),
    paragraphs(_paragraphs)
{
    perform_iteration();
}

void BMI_para::record_judgment(string doc_id, int judgment){
    record_judgment_batch({{doc_id.substr(0, doc_id.find(".")), judgment}});
}

vector<int> BMI_para::perform_training_iteration(){
    lock_guard<mutex> lock_training(training_mutex);

    {
        lock_guard<mutex> lock(training_cache_mutex);
        for(pair<int, int> training: training_cache){
            if(judgments.find(training.first) != judgments.end()){
                std::cerr<<"Rewriting judgment history"<<std::endl;
                if(judgments[training.first] > 0){
                    for(int i = (int)positives.size() - 1; i > 0; i--){
                        if(documents->get_index(positives[i]->doc_id) == training.first){
                            positives.erase(positives.begin() + i);
                            break;
                        }
                    }
                } else {
                    for(int i = (int)negatives.size() - 1; i >= random_negatives_index + random_negatives_size; i--){
                        if(documents->get_index(negatives[i]->doc_id) == training.first){
                            negatives.erase(negatives.begin() + i);
                            break;
                        }
                    }
                }
            }

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

            if(training.second > 0)
                positives.push_back(&documents->get_sf_sparse_vector(training.first));
            else
                negatives.push_back(&documents->get_sf_sparse_vector(training.first));
        }
        training_cache.clear();
    }

    // Training
    TIMER_BEGIN(training);
    auto weights = train();
    TIMER_END(training);

    // Scoring
    TIMER_BEGIN(rescoring);
    auto results = paragraphs->rescore(weights, num_threads,
                              judgments_per_iteration + (async_mode ? extra_judgment_docs : 100),
                              finished_judgments_para);
    TIMER_END(rescoring);

    return results;
}

std::vector<std::pair<string, float>> BMI_para::get_ranklist(){
    vector<std::pair<string, float>> ret_results;
    unordered_set<string> doc_id_seen;
    auto results = get_ranking_dataset()->rescore(train(), num_threads,
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

