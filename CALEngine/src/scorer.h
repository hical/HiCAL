#ifndef SCORER_H
#define SCORER_H

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <memory>
#include "sofiaml/sf-sparse-vector.h"

class Scorer {
    // This method will:
    // 1. Compute inner product of `weights` with each document from
    //    `doc_features[st] to doc_features[end]`
    // 2. Select top `num_top_docs` and place the respective {score, doc_id} in `top_docs`
    void score_docs(const std::vector<float> &weights, 
            int st,
            int end, 
            std::pair<float, int> *top_docs,
            int num_top_docs,
            const std::set<int> &judgments);

    public:
    // List of all document features
    std::shared_ptr<std::vector<std::unique_ptr<SfSparseVector>>> doc_features;

    // Number of features
    uint32_t dimensionality;

    // Inverted map of all document ids to their indices
    std::unordered_map<std::string, int> doc_ids_inv_map;

    // Construct the object using the feature file in svm light format
    Scorer(std::shared_ptr<std::vector<std::unique_ptr<SfSparseVector>>>);

    // Rescore all documents given the weights, and return the top documents
    void rescore_documents(const std::vector<float> &weights,
            int num_threads, 
            int top_docs_per_thread,
            const std::set<int> &judgments,
            std::vector<int> &top_docs_results);

    std::vector<std::pair<int, float>> rescore_all_documents(const std::vector<float> &weights, int num_threads);

    std::vector<std::pair<int, float>> get_top_terms(const std::vector<float> &weights, std::string doc_id, int num_top_terms);
};

#endif // SCORER_H
