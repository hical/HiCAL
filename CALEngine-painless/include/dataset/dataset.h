#ifndef DATASET_H
#define DATASET_H

#include "featurizer/featurizer.h"
#include "utils/sf-sparse-vector.h"
#include <cstddef>
#include <memory>
#include <string>

class Dataset {
protected:
  std::unique_ptr<Featurizer> featurizer_;

public:
  Dataset(std::unique_ptr<Featurizer> featurizer)
      : featurizer_(std::move(featurizer)) {}

  virtual void add_doc(const std::string &id, const std::string &text) = 0;

  virtual size_t size() = 0;

  virtual SfSparseVector get_features(const std::string &id) = 0;

  virtual SfSparseVector get_features(int id) = 0;
};

#endif // DATASET_H
