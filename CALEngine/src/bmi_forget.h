#ifndef BMI_FORGET_H
#define BMI_FORGET_H
#include <random>
#include <iostream>
#include <algorithm>
#include "bmi.h"
#include "classifier.h"

class BMI_forget:public BMI {
    std::unordered_map<const SfSparseVector*, int> judgment_order;
    const int num_remember;
    int cur_time_rel  = 0;
    int cur_time_nonrel  = 0;
    int full_train_period = -1;

    protected:
    virtual vector<float> train();
    public:
    BMI_forget(Seed _seed,
        Dataset *_documents,
        int _num_threads,
        int _judgments_per_iteration,
        bool _async_mode,
        int _num_remember,
        int _full_train_period,
        int _training_iterations)
    :BMI(_seed, _documents, _num_threads, _judgments_per_iteration, _async_mode, _training_iterations, false),
    num_remember(_num_remember), full_train_period(_full_train_period) {
        perform_iteration();
    }

    virtual void record_judgment(std::string doc_id, int judgment){
        size_t id = documents->get_index(doc_id);
        if(judgment > 0)
            judgment_order[&documents->get_sf_sparse_vector(id)] = ++cur_time_rel;
        else
            judgment_order[&documents->get_sf_sparse_vector(id)] = ++cur_time_nonrel;
        record_judgment_batch({{doc_id, judgment}});
    }
};

vector<float> BMI_forget::train(){
    // TODO fix this
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

    int cur_time = cur_time_rel + cur_time_nonrel;
    bool full_train = (full_train_period != -1 && (cur_time-1) % full_train_period == 0);

    for(const std::pair<int, int> &judgment: judgments){
        if(judgment.second > 0){
            if(full_train || judgment_order[&documents->get_sf_sparse_vector(judgment.first)] > cur_time_rel - num_remember)
                positives.push_back(&documents->get_sf_sparse_vector(judgment.first));
        }
        else{
            if(full_train || judgment_order[&documents->get_sf_sparse_vector(judgment.first)] > cur_time_nonrel - num_remember)
                negatives.push_back(&documents->get_sf_sparse_vector(judgment.first));
        }

    }

    std::cerr<<"Training on "<<positives.size()<<" +ve docs and "<<negatives.size()<<" -ve docs"<<std::endl;
    
    return LRPegasosClassifier(training_iterations).train(positives, negatives, documents->get_dimensionality());
}

#endif // BMI_FORGET_H
