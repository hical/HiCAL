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
    virtual vector<float> train();
    public:
    BMI_recency_weighting(Seed _seed,
        Dataset *_documents,
        int _num_threads,
        int _judgments_per_iteration,
        bool _async_mode,
        float _max_relative_weight,
        int _training_iterations)
    :BMI(_seed, _documents, _num_threads, _judgments_per_iteration, _async_mode, _training_iterations, false),
    max_relative_weight(_max_relative_weight) {
        perform_iteration();
    }

    virtual void record_judgment(std::string doc_id, int judgment){
        size_t id = documents->get_index(doc_id);
        judgment_order[&documents->get_sf_sparse_vector(id)] = ++cur_time;
        record_judgment_batch({{doc_id, judgment}});
    }
};

vector<float> BMI_recency_weighting::train(){
    std::uniform_int_distribution<size_t> distribution(0, documents->size()-1);
    for(int i = 0;i<random_negatives_size;i++){
        size_t idx = distribution(rand_generator);
        negatives[random_negatives_index + i] = &documents->get_sf_sparse_vector(idx);
    }

    std::cerr<<"Training on "<<positives.size()<<" +ve docs and "<<negatives.size()<<" -ve docs"<<std::endl;
    

    std::sort(positives.begin(), positives.end(), [this](const SfSparseVector *a, const SfSparseVector *b) -> bool {return this->judgment_order[a] < this->judgment_order[b];});
    std::sort(negatives.begin()+100, negatives.end(), [this](const SfSparseVector *a, const SfSparseVector *b) -> bool {return this->judgment_order[a] < this->judgment_order[b];});

    std::cerr<<"Training on "<<positives.size()<<" +ve docs and "<<negatives.size()<<" -ve docs"<<std::endl;
    
    return LRPegasosWeightedRecencyClassifier(max_relative_weight, training_iterations).train(positives, negatives, documents->get_dimensionality());
}

#endif // BMI_RECENCY_WEIGHTING_H
