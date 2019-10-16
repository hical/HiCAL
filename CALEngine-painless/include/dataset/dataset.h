#ifndef DATASET_H
#define DATASET_H

#include <cstddef>
#include <string>
#include <memory>
#include "utils/sf-sparse-vector.h"
#include "featurizer/featurizer.h"

class Dataset {
    std::unique_ptr<Featurizer> featurizer;

public:
    Dataset(std::unique_ptr<Featurizer> _featurizer): featurizer(std::move(_featurizer)) {}

    virtual size_t size() = 0;

    virtual SfSparseVector get_features(const std::string &id) = 0;

    virtual SfSparseVector get_features(int id) = 0;
};

#endif // DATASET_H
