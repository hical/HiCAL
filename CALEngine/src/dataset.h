#ifndef DATASET_H
#define DATASET_H

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <queue>
#include <mutex>
#include "sofiaml/sf-sparse-vector.h"

class Dataset {
    // List of all document features
    std::unique_ptr<std::vector<std::unique_ptr<SfSparseVector>>> doc_features;

    // Number of features
    const uint32_t dimensionality;

    // Inverted map of all document ids to their indices
    const std::unordered_map<std::string, size_t> doc_ids_inv_map;

    void score_docs_insertion_sort(const std::vector<float> &weights,
                                   int st, int end,
                                   std::pair<float, int> *top_docs,
                                   int num_top_docs,
                                   const std::map<int, int> &judgments);

    void score_docs_priority_queue(const std::vector<float> &weights,
                                   int st, int end,
                                   std::priority_queue<std::pair<float, int>> &top_docs,
                                   std::mutex &top_docs_mutex,
                                   int num_top_docs,
                                   const std::map<int, int> &judgments);

    public:
    uint32_t NPOS;
    Dataset(std::unique_ptr<std::vector<std::unique_ptr<SfSparseVector>>>);
    Dataset():doc_features(nullptr), dimensionality(0), NPOS(0){};

    // Returns the inner product of `weights` with the sparse vector at `index`
    virtual float inner_product(size_t index, const std::vector<float> &weights) const;

    // Returns the index given the document id. return Dataset::NPOS if not found
    size_t get_index(const std::string &id) const;

    virtual const SfSparseVector& get_sf_sparse_vector(size_t index) const {
        return *(doc_features->at(index));
    }

    virtual size_t size() const {
        return doc_features->size();
    }

    size_t get_dimensionality(){
        return dimensionality;
    }

    virtual int get_real_index(int id) const {return id;}
    std::vector<int> rescore(const vector<float> &weights, int num_threads, int num_top_docs, const std::map<int, int> &judgments);
};

class Dataset_subset:public Dataset {
    Dataset *d;
    std::vector<int> indices;
public:
    Dataset_subset(Dataset &_d, std::vector<int> _indices): d(&_d),indices(_indices) {
        NPOS = indices.size();
    }

    // Returns the inner product of `weights` with the sparse vector at `index`
    float inner_product(size_t index, const std::vector<float> &weights) const override {
        return d->inner_product(indices[index], weights);
    }

    const SfSparseVector& get_sf_sparse_vector(size_t index) const override {
        return d->get_sf_sparse_vector(indices[index]);
    }

    size_t size() const override {
        return indices.size();
    }

    int get_real_index(int id) const override {return indices[id];}
};

#endif // DATASET_H
