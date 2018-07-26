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
        ParagraphDataset *_paragraphs,
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
    sync_training_cache();

    // Training
    TIMER_BEGIN(training);
    auto weights = train();
    TIMER_END(training);

    // Scoring
    TIMER_BEGIN(rescoring);
    auto results = paragraphs->rescore(weights, num_threads,
                              judgments_per_iteration + (async_mode ? extra_judgment_docs : 0),
                              judgments);
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

