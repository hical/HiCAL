#ifndef SCORER_H
#define SCORER_H

#include<string>
#include<vector>
#include<set>
#include<unordered_map>
#include "sofiaml/sf-sparse-vector.h"

/* struct Feature{ */
/*     int id; */
/*     float weight; */
/* }; */

/* struct SparseFeatureVector{ */
/*     std::string doc_id; */
/*     std::vector<Feature> _vector; */

/*     SparseFeatureVector(std::string _doc_id, std::vector<Feature> __vector): */
/*     doc_id(_doc_id),_vector(__vector){} */
/* }; */

extern std::vector<SfSparseVector> doc_features;
extern std::unordered_map<std::string, int> doc_ids_inv_map;

std::vector<SfSparseVector> parse_doc_features(std::string fname);
std::vector<float> parse_model_file(std::string fname);
extern int dimensionality;

void score_docs(const std::vector<float> &weights, 
        int st,
        int end, 
        std::pair<float, int> *top_docs,
        int num_top_docs,
        const std::set<int> &judgments);

void rescore_documents(const std::vector<float> &weights,
        int num_threads, 
        int top_docs_per_thread,
        const std::set<int> &judgments,
        std::vector<int> &top_docs_results);

void set_doc_features(std::string fname);

#endif
