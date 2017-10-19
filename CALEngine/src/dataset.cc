#include "dataset.h"

using namespace std;

typedef unique_ptr<vector<unique_ptr<SfSparseVector>>> SparseVectors;

unordered_map<string, size_t> generate_inverted_index(const SparseVectors &sparse_vectors) {
    unordered_map<string, size_t> inverted_index;
    for(size_t i = 0; i < sparse_vectors->size(); i++){
        inverted_index[sparse_vectors->at(i)->doc_id] = i;
    }
    return inverted_index;
}

uint32_t compute_dimensionality(const SparseVectors &sparse_vectors) {
    uint32_t dimensionality = 0;
    for(size_t i = 0; i < sparse_vectors->size(); i++){
        for(auto feature: sparse_vectors->at(i)->features_)
            dimensionality = max(dimensionality, feature.id_);
    }
    return 1 + dimensionality;
}

Dataset::Dataset(SparseVectors sparse_vectors):
dimensionality(compute_dimensionality(sparse_vectors)),
NPOS(sparse_vectors->size()),
doc_ids_inv_map(generate_inverted_index(sparse_vectors)) {
    doc_features = move(sparse_vectors);
}

size_t Dataset::get_index(const std::string &id) const {
    auto result = doc_ids_inv_map.find(id);
    if ( result == doc_ids_inv_map.end() )
        return NPOS;
    return result->second;
}
