#ifndef DATASET_MEMORY_H
#define DATASET_MEMORY_H

#include <unordered_map>

#include "dataset/dataset.h"

class DatasetMemory : public Dataset {
    std::vector<SfSparseVector> docs_;
    std::unordered_map<std::string, int> doc_index_;

    void load_from_svmlight(const std::string &filename);
    void load_from_bin(const std::string &filename);
    void write_to_svmlight(const std::string &filename) const;
    void write_to_bin(const std::string &filename) const;

   public:
    enum DatasetFormat { SVMLIGHT, BIN };

    explicit DatasetMemory(std::unique_ptr<Featurizer> featurizer);

    explicit DatasetMemory(std::unique_ptr<Featurizer> featurizer,
                           const std::string &filename,
                           DatasetFormat dataset_format);

    virtual size_t size() const override;

    virtual const SfSparseVector *transform(
        const std::string &id) const override;

    virtual const SfSparseVector *transform(int id) const override;

    virtual void add_doc(const std::string &id,
                         const std::string &text) override;

    void write(const std::string &filename, DatasetFormat dataset_format) const;
};

#endif  // DATASET_MEMORY_H
