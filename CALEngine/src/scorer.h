#ifndef SCORER_H
#define SCORER_H

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <memory>
#include <queue>
#include <mutex>
#include "sofiaml/sf-sparse-vector.h"
#include "dataset.h"

namespace Scorer {
    // This method will:
    // 1. Compute inner product of `weights` with each document from
    //    `doc_features[st] to doc_features[end]`
    // 2. Select top `num_top_docs` and place the respective {score, doc_id} in `top_docs`
    void score_docs_insertion_sort(const Dataset &dataset,
            const std::vector<float> &weights,
            int st,
            int end, 
            std::pair<float, int> *top_docs,
            int num_top_docs,
            const std::set<int> &judgments);

    void score_docs_priority_queue(const Dataset &dataset,
                                   const std::vector<float> &weights,
                                   int st,
                                   int end,
                                   std::priority_queue<std::pair<float, int>> &top_docs,
                                   std::mutex &top_docs_mutex,
                                   int num_top_docs,
                                   const std::set<int> &judgments);

    // Rescore all documents given the weights, and return the K top documents
    void rescore_documents(const Dataset &dataset,
            const std::vector<float> &weights,
            int num_threads,
            int K,
            const std::set<int> &judgments,
            std::vector<int> &top_docs_results);

    std::vector<std::pair<int, float>> rescore_all_documents(const Dataset &, const std::vector<float> &weights, int num_threads);

};

#endif // SCORER_H
