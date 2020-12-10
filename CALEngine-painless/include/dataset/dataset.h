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
    struct DatasetItem {
        std::string id;
        SfSparseVector featureVector;
    };

    class IteratorBase {
       public:
        virtual std::unique_ptr<IteratorBase> next() const = 0;
        virtual const DatasetItem &getItem() const = 0;
        virtual bool equals(const IteratorBase &it) const = 0;
    };

    class Iterator {
        /**
         * The `baseIterator` is how we achieve what a polymorphic iterator
         * would have achieved. The type has to be a shared_ptr because we want
         * the iterator to be a copyable.
         */
        std::shared_ptr<IteratorBase> baseIterator_;

       public:
        explicit Iterator(std::unique_ptr<IteratorBase> baseIt)
            : baseIterator_(std::move(baseIt)) {}

        Iterator operator++() {
            auto nextIterator = baseIterator_->next();
            baseIterator_.reset(nextIterator.release());
            return *this;
        }

        Iterator operator++(int) {
            auto cur = *this;
            auto nextIterator = baseIterator_->next();
            baseIterator_.reset(nextIterator.release());
            return cur;
        }

        bool operator==(const Iterator &it) const {
            return typeid(*this) == typeid(it) &&
                   baseIterator_->equals(*it.baseIterator_);
        }

        bool operator!=(const Iterator &it) const { return !(*this == it); }

        auto &operator*() { return baseIterator_->getItem(); }
    };

    explicit Dataset(std::unique_ptr<Featurizer> featurizer)
        : featurizer_(std::move(featurizer)) {}

    virtual void add(const std::string &id, const std::string &text) = 0;
    virtual size_t size() const = 0;
    virtual const DatasetItem &get(const std::string &id) const = 0;
    const Featurizer &featurizer() const { return *featurizer_; }

    // Iterators
    virtual Iterator begin() const = 0;
    virtual Iterator end() const = 0;
};

#endif  // DATASET_H
