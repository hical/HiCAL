#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <random>
#include <vector>
#include <iostream>
#include "sofiaml/sf-sparse-vector.h"

static thread_local std::mt19937 rand_generator;

class Classifier {
    public:
    virtual vector<float> train(const std::vector<const SfSparseVector*> &positives,
               const std::vector<const SfSparseVector*> &negatives, int dimensionality) = 0;
};

class LRPegasosClassifier:public Classifier {
    const float lambda = 0.0001;
    const int num_iters;

    protected:
    virtual int RandInt(int num_vals) {
        std::uniform_int_distribution<int> distribution(0, num_vals-1);
        return distribution(rand_generator);
    }

    public:
    LRPegasosClassifier(int _num_iters):num_iters(_num_iters) {}

    virtual vector<float> train(const std::vector<const SfSparseVector*> &positives,
               const std::vector<const SfSparseVector*> &negatives, int dimensionality);
};

class LRPegasosWeightedRecencyClassifier:public LRPegasosClassifier {
    int c;
    protected:
    virtual int RandInt(int num_vals) {
        num_vals--;
        if(num_vals == 0)return 0;
        double m = (c-1)/double(num_vals);
        double new_max = (m*(num_vals+1)*(num_vals+1) - m*(num_vals+1) + 2*(num_vals+1))/2;
        std::uniform_real_distribution<double> distribution(0, new_max);
        double r = distribution(rand_generator);
        if(m < 1e-10)
            return int(r);
        double r_2 = (m-2 + sqrt((2-m)*(2-m)+8*r*m))/(2*m);
        return int(r_2);
    }

    public:
    LRPegasosWeightedRecencyClassifier(int _c, int _num_iters):c(_c), LRPegasosClassifier(_num_iters){}
};

#endif // CLASSIFIER_H
