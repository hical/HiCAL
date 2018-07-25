#include <thread>
#include "dataset.h"
#include "utils/utils.h"

using namespace std;

typedef unique_ptr<vector<unique_ptr<SfSparseVector>>> SparseVectors;

unordered_map<string, size_t> generate_inverted_index(const SparseVectors &sparse_vectors) {
    unordered_map<string, size_t> inverted_index;
    for(size_t i = 0; i < sparse_vectors->size(); i++){
        inverted_index[sparse_vectors->at(i)->doc_id] = i;
    }
    return inverted_index;
}

vector<int> generate_parent_documents(const Dataset &parent_dataset, const SparseVectors &sparse_vectors){
    vector<int> parent_documents(sparse_vectors->size());
    for(int i = 0; i < parent_documents.size(); i++){
        auto &para_id = sparse_vectors->at(i)->doc_id;
        string doc_id = para_id.substr(0, para_id.find("."));
        parent_documents[i] = parent_dataset.get_index(doc_id);
        
        if(i > 0 && parent_documents[i] < parent_documents[i-1]){
            fail("Paragraphs must be in increasing order of their parent document ids", -1);
        }
    }
    return parent_documents;
}

uint32_t compute_dimensionality(const SparseVectors &sparse_vectors) {
    uint32_t dimensionality = 0;
    for (auto &spv : *sparse_vectors) {
        for(auto feature: spv->features_)
            dimensionality = max(dimensionality, feature.id_);
    }
    return 1 + dimensionality;
}

Dataset::Dataset(SparseVectors sparse_vectors, std::unordered_map<std::string, TermInfo> _dictionary):
dictionary(_dictionary),
dimensionality(compute_dimensionality(sparse_vectors)),
doc_ids_inv_map(generate_inverted_index(sparse_vectors)),
NPOS(sparse_vectors->size())
{
    doc_features = move(sparse_vectors);
}

float Dataset::inner_product(size_t index, const vector<float> &weights) const {
    auto &features = get_sf_sparse_vector(index).features_;
    float score = 0;
    for(auto &feature: features){
        score += weights[feature.id_] * feature.value_;
    }
    return score;
}

void Dataset::score_docs_priority_queue(const vector<float> &weights,
                                       int st, int end,
                                       priority_queue<pair<float, int>> &top_docs,
                                       mutex &top_docs_mutex,
                                       int num_top_docs,
                                       const map<int, int> &judgments) {
    auto iterator = judgments.lower_bound(translate_index(st));
    pair<float, int> buffer[1000];
    int buffer_idx = 0;
    for(int i = st;i<end; i++){
        while(iterator != judgments.end() && iterator->first < translate_index(i))
            iterator++;
        if(!(iterator != judgments.end() && iterator->first == translate_index(i))){
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
    priority_queue<pair<float, int>> top_docs;

    // Fix the last segment to contain everything remaining
    for(int i = 0; i< num_threads;i++){
        t.push_back(
            thread(
                &Dataset::score_docs_priority_queue,
                this,
                cref(weights),
                i * this->size()/num_threads,
                (i == num_threads - 1)?this->size():(i+1) * this->size()/num_threads,
                ref(top_docs),
                ref(top_docs_mutex),
                num_top_docs,
                ref(judgments)
            )
        );
    }
    for(thread &x: t)
        x.join();

    vector<int> top_docs_list(top_docs.size());
    int idx = 0;
    while(!top_docs.empty()){
        top_docs_list[idx++] = (top_docs.top().second);
        top_docs.pop();
    }
    return top_docs_list;
}

ParagraphDataset::ParagraphDataset(const Dataset &_parent_dataset,
        SparseVectors sparse_vectors,
        std::unordered_map<std::string, TermInfo> _dictionary):
            Dataset(move(sparse_vectors), _dictionary),
            parent_dataset(_parent_dataset){
    parent_documents = generate_parent_documents(_parent_dataset, doc_features);
}
