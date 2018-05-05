#include <iostream>
#include "bmi_online_learning.h"
using namespace std;

BMI_online_learning::BMI_online_learning(Seed _seed,
        Dataset *_documents,
        int _num_threads,
        int _judgments_per_iteration,
        bool _async_mode,
        size_t _refresh_period,
        float _delta,
        int _training_iterations)
    :BMI(_seed, _documents, _num_threads, _judgments_per_iteration, _async_mode, _training_iterations, false),
    refresh_period(_refresh_period), delta(_delta)
{
    perform_iteration();
}
vector<int> BMI_online_learning::perform_training_iteration(){
    lock_guard<mutex> lock_training(training_mutex);

    {
        lock_guard<mutex> lock(training_cache_mutex);
        for(pair<int, int> training: training_cache){
            if(!is_it_refresh_time()){
                float p = 1 / (1 + exp(-documents->inner_product(training.first, this->weight)));
                int is_rel = (training.second > 0);
                for(FeatureValuePair feature: documents->get_sf_sparse_vector(training.first).features_){
                    this->weight[feature.id_] += (is_rel - p) * delta;
                }
            }
            judgments[training.first] = training.second;
        }
        training_cache.clear();
    }

    // Training

    if(is_it_refresh_time()){
        auto start = std::chrono::steady_clock::now();
        this->weight = train();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Training finished in "<<duration.count()<<"ms"<<endl;
    }



    // Scoring
    auto start = std::chrono::steady_clock::now();
    auto results = documents->rescore(this->weight, num_threads,
                              judgments_per_iteration + (async_mode ? extra_judgment_docs : 0), judgments);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Rescored "<<documents->size()<<" documents in "<<duration.count()<<"ms"<<endl;

    return results;
}
