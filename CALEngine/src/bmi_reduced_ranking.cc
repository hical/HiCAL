#include <iostream>
#include <algorithm>
#include <thread>
#include "bmi_reduced_ranking.h"
#include "utils/utils.h"
using namespace std;

BMI_reduced_ranking::BMI_reduced_ranking(Seed _seed,
        Dataset *_documents,
        int _num_threads,
        int _judgments_per_iteration,
        bool _async_mode,
        size_t _subset_size,
        size_t _refresh_period,
        int _training_iterations)
    :BMI(_seed, _documents, _num_threads, _judgments_per_iteration, _async_mode, _training_iterations, false),
    subset_size(_subset_size), refresh_period(_refresh_period)
{
    perform_iteration();
}

void BMI_reduced_ranking::score_docs_insertion_sort(const vector<float> &weights,
                               int st, int end,
                               pair<float, int> *top_docs) {
    for(int i = st;i<end; i++){
        if(judgments.find(indices[i]) != judgments.end())
            continue;

        float score = documents->inner_product(indices[i], weights);

        int idx = judgments_per_iteration-1;
        while(idx >= 0 && top_docs[idx].first < score){
            if(idx != judgments_per_iteration - 1){
                top_docs[idx+1] = top_docs[idx];
            }
            top_docs[idx] = {score, indices[i]};
            idx--;
        }
    }
}

vector<int> BMI_reduced_ranking::subset_rescore(const vector<float> &weights) {
    vector<int> res;
    vector<thread> t;

    pair<float, int> top_docs[judgments_per_iteration * num_threads];
    for(int i = 0;i<judgments_per_iteration * num_threads; i++)
        top_docs[i] = {-1e9, -1};
    
    // Fix the last segment to contain everything remaining
    for(int i = 0; i< num_threads;i++){
        t.push_back(
            thread(
                &BMI_reduced_ranking::score_docs_insertion_sort,
                this,
                cref(weights),
                i * indices.size()/num_threads,
                (i == num_threads - 1)?indices.size():(i+1) * indices.size()/num_threads,
                top_docs + judgments_per_iteration * i
            )
        );
    }

    for(thread &x: t)
        x.join();

    sort(top_docs, top_docs + judgments_per_iteration * num_threads, greater<pair<float, int>>());
    for(int i = judgments_per_iteration-1;i>=0; i--){
        res.push_back(top_docs[i].second);
    }
    return res;
}

vector<int> BMI_reduced_ranking::perform_training_iteration(){
    lock_guard<mutex> lock_training(training_mutex);

    sync_training_cache();

    // Training
    TIMER_BEGIN(training);
    auto weights = train();
    TIMER_END(training);

    // Scoring
    if(is_it_refresh_time()){
        TIMER_BEGIN(rescoring);
        auto results = documents->rescore(weights, num_threads, subset_size, judgments);
        TIMER_END(rescoring);

        indices.clear();
        for(int i: results)
            indices.push_back(i);
        sort(indices.begin(), indices.end());

        return results;
    } else {
        TIMER_BEGIN(partial_rescoring);
        auto results = subset_rescore(weights);
        TIMER_END(partial_rescoring);

        return results;
    }
}
