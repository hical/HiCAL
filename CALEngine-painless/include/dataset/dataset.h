#ifndef DATASET_H
#define DATASET_H

#include <cstddef>
#include <memory>
#include <string>

#include "featurizer/featurizer.h"
#include "utils/sf-sparse-vector.h"

class Dataset {
   protected:
    std::unique_ptr<Featurizer> featurizer_;

   public:
    Dataset(std::unique_ptr<Featurizer> featurizer)
        : featurizer_(std::move(featurizer)) {}

    virtual void add_doc(const std::string &id, const std::string &text) = 0;

    virtual size_t size() const = 0;

    virtual const SfSparseVector *transform(const std::string &id) const = 0;

    virtual const SfSparseVector *transform(int id) const = 0;

    const Featurizer &get_featurizer() const { return *featurizer_; }
};

#endif  // DATASET_H
