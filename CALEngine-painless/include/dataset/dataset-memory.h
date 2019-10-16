#ifndef DATASET_MEMORY_H
#define DATASET_MEMORY_H

#include "dataset/dataset.h"

class DatasetMemory : public Dataset {

public:

  explicit DatasetMemory(std::unique_ptr<Featurizer> _featurizer,
                         std::string filename);

  static DatasetMemory create(const std::string &filename);

  virtual size_t size();

  virtual SfSparseVector get_features(const std::string &id);

  virtual SfSparseVector get_features(int id);
};

#endif // DATASET_MEMORY_H
