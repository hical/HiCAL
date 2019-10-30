#include <thread>
#include <algorithm>
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

string get_doc_id(const string &para_id){
    return para_id.substr(0, para_id.find("."));
}

vector<int> generate_parent_documents(const Dataset &parent_dataset, const SparseVectors &sparse_vectors){
    vector<int> parent_documents(sparse_vectors->size());
    for(int i = 0; i < parent_documents.size(); i++){
        parent_documents[i] = parent_dataset.get_index(get_doc_id(sparse_vectors->at(i)->doc_id));
        
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

void ParagraphDataset::score_docs_priority_queue(const vector<float> &weights,
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

            if(i == st || translate_index(i) != translate_index(i-1))
                buffer[buffer_idx] = {-score, i};
            else
                buffer[buffer_idx] = min(buffer[buffer_idx], {-score, i});
            
            if(i == end - 1 || translate_index(i) != translate_index(i+1))
                buffer_idx++;
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

    for(thread &x: t) x.join();

    vector<int> top_docs_list(top_docs.size());
    int idx = 0;
    while(!top_docs.empty()){
        top_docs_list[idx++] = (top_docs.top().second);
        top_docs.pop();
    }
    return top_docs_list;
}

vector<int> ParagraphDataset::rescore(const vector<float> &weights, int num_threads, int num_top_docs, const map<int, int> &judgments) {
    vector<thread> t;
    mutex top_docs_mutex;
    priority_queue<pair<float, int>> top_docs;

    uint32_t prev_end;
    for(int i = 0; i < num_threads;i++){
        int start = i * this->size()/num_threads;
        
        if(i != 0) start = prev_end;
        prev_end = (i == num_threads - 1)?this->size():(i+1) * this->size()/num_threads;
        while(prev_end > 0 && prev_end < this->size() && translate_index(prev_end) == translate_index(prev_end - 1))
            prev_end++;

        t.push_back(
            thread(
                &ParagraphDataset::score_docs_priority_queue,
                this,
                cref(weights),
                start, prev_end,
                ref(top_docs),
                ref(top_docs_mutex),
                num_top_docs,
                ref(judgments)
            )
        );
    }

    for(thread &x: t) x.join();

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
    sort(
        doc_features->begin(),
        doc_features->end(),
        [&](const unique_ptr<SfSparseVector> &a, const unique_ptr<SfSparseVector> &b) -> bool {
            return parent_dataset.get_index(get_doc_id(a->doc_id)) < parent_dataset.get_index(get_doc_id(b->doc_id));
        }
    );
    parent_documents = generate_parent_documents(_parent_dataset, doc_features);
}
