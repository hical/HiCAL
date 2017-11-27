#include <thread>
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
    for (auto &spv : *sparse_vectors) {
        for(auto feature: spv->features_)
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

size_t Dataset::get_index(const string &id) const {
    auto result = doc_ids_inv_map.find(id);
    if ( result == doc_ids_inv_map.end() )
        return NPOS;
    return result->second;
}

float Dataset::inner_product(size_t index, const vector<float> &weights) const {
    auto &features = get_sf_sparse_vector(index).features_;
    float score = 0;
    for(auto &feature: features){
        score += weights[feature.id_] * feature.value_;
    }
    return score;
//    vector<float> a, b;
//    for(auto &feature: features){
//        a.push_back(weights[feature.id_]);
//        b.push_back(feature.value_);
//    }
//    float score1 = 0, score2 = 0, score3 = 0, score4 = 0;
//    for(int i = 3; i < features.size(); i+=4){
//        score1 += weights[features[i].id_] * features[i].value_;
//        score2 += weights[features[i-1].id_] * features[i-1].value_;
//        score3 += weights[features[i-2].id_] * features[i-2].value_;
//        score4 += weights[features[i-3].id_] * features[i-3].value_;
//    }
//    return score1 + score2 + score3 + score4;
}

void Dataset::score_docs_insertion_sort(const vector<float> &weights,
                               int st, int end,
                               pair<float, int> *top_docs,
                               int num_top_docs,
                               const map<int, int> &judgments) {
    auto iterator = judgments.lower_bound(st);
    for(int i = st;i<end; i++){
        while(iterator != judgments.end() && iterator->first < i)
            iterator++;
        if(iterator != judgments.end() && iterator->first == i)
            continue;

        float score = this->inner_product(i, weights);

        int idx = num_top_docs-1;
        while(idx >= 0 && top_docs[idx].first < score){
            if(idx != num_top_docs - 1){
                top_docs[idx+1] = top_docs[idx];
            }
            top_docs[idx] = {score, i};
            idx--;
        }
    }
}

void Dataset::score_docs_priority_queue(const vector<float> &weights,
                                       int st, int end,
                                       priority_queue<pair<float, int>> &top_docs,
                                       mutex &top_docs_mutex,
                                       int num_top_docs,
                                       const map<int, int> &judgments) {
    auto iterator = judgments.lower_bound(st);
    pair<float, int> buffer[1000];
    int buffer_idx = 0;
    for(int i = st;i<end; i++){
        while(iterator != judgments.end() && iterator->first < get_real_index(i))
            iterator++;
        if(!(iterator != judgments.end() && iterator->first == get_real_index(i))){
            float score = this->inner_product(i, weights);
            buffer[buffer_idx++] = {-score, i};
        }

        if(buffer_idx == 1000 || i == end - 1){
            lock_guard<mutex> lock(top_docs_mutex);
            for(int j = 0;j < buffer_idx; j++){
                if(top_docs.size() < num_top_docs)
                    top_docs.push(buffer[j]);
                else if(-buffer[j].first > -top_docs.top().first){
                    top_docs.pop();
                    top_docs.push(buffer[j]);
                }
            }
            buffer_idx = 0;
        }
    }
}

vector<int> Dataset::rescore(const vector<float> &weights, int num_threads, int num_top_docs, const map<int, int> &judgments) {
    vector<thread> t;
    mutex top_docs_mutex;
    auto *top_docs = new priority_queue<pair<float, int>>;

    // Fix the last segment to contain everything remaining
    for(int i = 0; i< num_threads;i++){
        t.push_back(
            thread(
                &Dataset::score_docs_priority_queue,
                this,
                cref(weights),
                i * this->size()/num_threads,
                (i == num_threads - 1)?this->size():(i+1) * this->size()/num_threads,
                ref(*top_docs),
                ref(top_docs_mutex),
                num_top_docs,
                ref(judgments)
            )
        );
    }
    for(thread &x: t)
        x.join();

    vector<int> top_docs_list(top_docs->size());
    while(!top_docs->empty()){
        top_docs_list[top_docs->size()-1] = (top_docs->top().second);
        top_docs->pop();
    }
    delete top_docs;
    return top_docs_list;
}
