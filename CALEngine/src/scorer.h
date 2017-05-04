#ifndef SCORER_H
#define SCORER_H

#include<string>
#include<vector>
#include<unordered_map>

struct Feature{
    int id;
    float weight;
};

struct SparseFeatureVector{
    std::string doc_id;
    std::vector<Feature> _vector;

    SparseFeatureVector(std::string _doc_id, std::vector<Feature> __vector):
    doc_id(_doc_id),_vector(__vector){}
};

extern std::vector<SparseFeatureVector> doc_features;
extern std::unordered_map<std::string, int> doc_ids_inv_map;

std::vector<SparseFeatureVector> parse_doc_features(std::string fname);

std::vector<float> parse_model_file(std::string fname);

void score_docs(const std::vector<float> &weights, 
        int st,
        int end, 
        std::pair<float, int> *top_docs,
        int num_top_docs,
        const std::vector<int> &judgments);

void rescore_documents(const std::vector<float> &weights,
        int num_threads, 
        int top_docs_per_thread,
        const std::vector<int> &judgments,
        std::vector<int> &top_docs_results);

void set_doc_features(std::string fname);

#endif
