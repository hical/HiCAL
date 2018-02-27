#ifndef BMI_RECENCY_WEIGHTING_H
#define BMI_RECENCY_WEIGHTING_H
#include <random>
#include <iostream>
#include <algorithm>
#include "bmi.h"
#include "classifier.h"

class BMI_recency_weighting:public BMI {
    std::unordered_map<const SfSparseVector*, int> judgment_order;
    const float max_relative_weight;
    int cur_time  = 0;
    protected:
    virtual SfWeightVector train();
    public:
    BMI_recency_weighting(Seed _seed,
        Dataset *_documents,
        int _num_threads,
        int _judgments_per_iteration,
        int _max_effort,
        int _max_iterations,
        bool _async_mode,
        float _max_relative_weight)
    :BMI(_seed, _documents, _num_threads, _judgments_per_iteration, _max_effort, _max_iterations, _async_mode, false),
    max_relative_weight(_max_relative_weight) {
        perform_iteration();
    }

    virtual void record_judgment(std::string doc_id, int judgment){
        record_judgment_batch({{doc_id, judgment}});
        size_t id = documents->get_index(doc_id);
        judgment_order[&documents->get_sf_sparse_vector(id)] = ++cur_time;
    }
};

SfWeightVector BMI_recency_weighting::train(){
    SfWeightVector w(documents->get_dimensionality());
    vector<const SfSparseVector*> positives, negatives;
    for(auto &judgment: seed){
        if(judgment.second > 0)
            positives.push_back(&judgment.first);
        else
            negatives.push_back(&judgment.first);
    }

    // Sampling random non_rel documents
    std::uniform_int_distribution<size_t> distribution(0, documents->size()-1);
    for(int i = 1;i<=100;i++){
        size_t idx = distribution(rand_generator);
        negatives.push_back(&documents->get_sf_sparse_vector(idx));
    }

    for(const std::pair<int, int> &judgment: judgments){
        if(judgment.second > 0)
            positives.push_back(&documents->get_sf_sparse_vector(judgment.first));
        else
            negatives.push_back(&documents->get_sf_sparse_vector(judgment.first));
    }

    std::sort(positives.begin(), positives.end(), [this](const SfSparseVector *a, const SfSparseVector *b) -> bool {return this->judgment_order[a] < this->judgment_order[b];});

    std::cerr<<"Training on "<<positives.size()<<" +ve docs and "<<negatives.size()<<" -ve docs"<<std::endl;
    
    LRPegasosWeightedRecencyClassifier(max_relative_weight).train(positives, negatives, &w);
    return w;
}

#endif // BMI_RECENCY_WEIGHTING_H
