#ifndef DATASET_MEMORY_H
#define DATASET_MEMORY_H

#include <unordered_map>
#include "dataset/dataset.h"

class DatasetMemory : public Dataset {
    std::vector<SfSparseVector> docs_;
    std::unordered_map<std::string, int> doc_index_;

  void load_from_svmlight(const std::string &filename);
  void load_from_bin(const std::string &filename);
  void write_to_svmlight(const std::string &filename);
  void write_to_bin(const std::string &filename);

public:
  enum DatasetFormat { SVMLIGHT, BIN };

  explicit DatasetMemory(std::unique_ptr<Featurizer> featurizer);

  explicit DatasetMemory(std::unique_ptr<Featurizer> featurizer,
                         const std::string &filename,
                         DatasetFormat dataset_format);

  virtual size_t size() const;

  virtual const SfSparseVector* get_features(const std::string &id) const;

  virtual const SfSparseVector* get_features(int id) const;

  virtual void add_doc(const std::string &id, const std::string &text);

  virtual void write(const std::string &filename, DatasetFormat dataset_format);
};

#endif // DATASET_MEMORY_H
