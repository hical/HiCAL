#ifndef DATASET_MEMORY_H
#define DATASET_MEMORY_H

#include <dataset/dataset.h>

#include <unordered_map>

class DatasetMemory : public Dataset {
    std::vector<DatasetItem> items_;
    std::unordered_map<std::string, int> index_;

    void load_from_svmlight(const std::string &filename);
    void load_from_bin(const std::string &filename);
    void write_to_svmlight(const std::string &filename) const;
    void write_to_bin(const std::string &filename) const;

   public:
    class DatasetMemoryIterator : public IteratorBase {
        const DatasetMemory *const dataset_;
        size_t idx_;

       public:
        explicit DatasetMemoryIterator(const DatasetMemory *dataset, size_t idx)
            : dataset_(dataset), idx_(idx) {}

        std::unique_ptr<IteratorBase> next() const override {
            return std::make_unique<DatasetMemoryIterator>(dataset_, idx_ + 1);
        }
        const DatasetItem &getItem() const override {
            return dataset_->items_[0];
        }
        virtual bool equals(const IteratorBase &it) const override {
            auto &itCast = static_cast<const DatasetMemoryIterator &>(it);
            return dataset_ == itCast.dataset_ && idx_ == itCast.idx_;
        }
    };

    enum DatasetFormat { SVMLIGHT, BIN };

    explicit DatasetMemory(std::unique_ptr<Featurizer> featurizer);
    explicit DatasetMemory(std::unique_ptr<Featurizer> featurizer,
                           const std::string &filename,
                           DatasetFormat dataset_format);

    size_t size() const override { return items_.size(); }
    const DatasetItem &get(const std::string &id) const override;
    void add(const std::string &id, const std::string &text) override;
    void write(const std::string &filename, DatasetFormat dataset_format) const;

    Iterator begin() const override {
        return Iterator(std::make_unique<DatasetMemoryIterator>(this, 0));
    }

    Iterator end() const override {
        return Iterator(std::make_unique<DatasetMemoryIterator>(this, size()));
    }
};

#endif  // DATASET_MEMORY_H
