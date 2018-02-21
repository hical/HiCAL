#include <iostream>
#include <algorithm>
#include "bmi_reduced_ranking.h"
using namespace std;

BMI_reduced_ranking::BMI_reduced_ranking(Seed _seed,
        Dataset *_documents,
        int _num_threads,
        int _judgments_per_iteration,
        int _max_effort,
        int _max_iterations,
        bool _async_mode,
        size_t _subset_size,
        size_t _refresh_period)
    :BMI(_seed, _documents, _num_threads, _judgments_per_iteration, _max_effort, _max_iterations, _async_mode, false),
    subset_size(_subset_size), refresh_period(_refresh_period)
{
    perform_iteration();
}

vector<int> BMI_reduced_ranking::perform_training_iteration(){
    lock_guard<mutex> lock_training(training_mutex);

    {
        lock_guard<mutex> lock(training_cache_mutex);
        for(pair<int, int> training: training_cache){
            judgments[training.first] = training.second;
        }
        training_cache.clear();
    }

    // Training
    auto start = std::chrono::steady_clock::now();

    auto w = train();
    auto weights = w.AsFloatVector();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Training finished in "<<duration.count()<<"ms"<<endl;

    // Scoring
    if(is_it_refresh_time()){
        start = std::chrono::steady_clock::now();
        auto results = documents->rescore(weights, num_threads, subset_size, judgments);
        duration = std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::steady_clock::now() - start);
        cerr<<"Rescored "<<documents->size()<<" documents in "<<duration.count()<<"ms"<<endl;

        indices.clear();
        for(int i: results)
            indices.push_back(i);
        sort(indices.begin(), indices.end());

        delete subset;
        subset = new Dataset_subset(*documents, indices);

        return results;
    } else {
        start = std::chrono::steady_clock::now();
        auto results = subset->rescore(weights, num_threads, judgments_per_iteration+(async_mode?extra_judgment_docs:0), judgments);
        duration = std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::steady_clock::now() - start);
        cerr<<"Rescored "<<subset->size()<<" documents in "<<duration.count()<<"ms"<<endl;

        for(auto &i: results)
            i = indices[i];

        return results;
    }
}
