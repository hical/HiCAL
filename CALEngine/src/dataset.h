#ifndef DATASET_H
#define DATASET_H

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include "sofiaml/sf-sparse-vector.h"

class Dataset {
    // Number of features
    const uint32_t dimensionality;

    // Inverted map of all document ids to their indices
    const std::unordered_map<std::string, size_t> doc_ids_inv_map;

    // List of all document features
    std::unique_ptr<std::vector<std::unique_ptr<SfSparseVector>>> doc_features;
    
    public:
    const uint32_t NPOS;
    Dataset(std::unique_ptr<std::vector<std::unique_ptr<SfSparseVector>>>);

    // Returns the inner product of `weights` with the sparse vector at `index`
    float inner_product(size_t index, const std::vector<float> &weights) const;

    // Returns the index given the document id. return Dataset::NPOS if not found
    size_t get_index(const std::string &id) const;

    const SfSparseVector& get_sf_sparse_vector(size_t index) const {
        return *(doc_features->at(index));
    }

    size_t size() const {
        return doc_features->size();
    }

    size_t get_dimensionality(){
        return dimensionality;
    }
};

#endif // DATASET_H
