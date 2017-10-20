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

float Dataset::inner_product(size_t index, const std::vector<float> &weights) const {
    auto &features = get_sf_sparse_vector(index).features_;
    float score = 0;
    for(auto &feature: features){
        score += weights[feature.id_] * feature.value_;
    }
    return score;
//    float score1 = 0, score2 = 0, score3 = 0, score4 = 0;
//    for(int i = 3; i < features.size(); i+=4){
//        score1 += weights[features[i].id_] * features[i].value_;
//        score2 += weights[features[i-1].id_] * features[i-1].value_;
//        score3 += weights[features[i-2].id_] * features[i-2].value_;
//        score4 += weights[features[i-3].id_] * features[i-3].value_;
//    }
//    return score1 + score2 + score3 + score4;
}
