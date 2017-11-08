#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <map>
#include "scorer.h"

using namespace std;

void Scorer::score_docs_insertion_sort(
        const Dataset &dataset,
        const vector<float> &weights,
        int st,
        int end,
        pair<float, int> *top_docs,
        int num_top_docs,
        const map<int, int> &judgments) {
    auto iterator = judgments.lower_bound(st);
    for(int i = st;i<end; i++){
        while(iterator != judgments.end() && iterator->first < i)
            iterator++;
        if(iterator != judgments.end() && iterator->first == i)
            continue;

        float score = dataset.inner_product(i, weights);

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

void Scorer::score_docs_priority_queue(const Dataset &dataset, const std::vector<float> &weights, int st, int end,
                                       std::priority_queue<std::pair<float, int>> &top_docs, std::mutex &top_docs_mutex,
                                       int num_top_docs, const map<int, int> &judgments) {
    auto iterator = judgments.lower_bound(st);
    pair<float, int> buffer[1000];
    int buffer_idx = 0;
    for(int i = st;i<end; i++){
        while(iterator != judgments.end() && iterator->first < i)
            iterator++;
        if(iterator != judgments.end() && iterator->first == i)
            continue;

        float score = dataset.inner_product(i, weights);
        buffer[buffer_idx++] = {dataset.inner_product(i, weights), i};
        if(buffer_idx == 1000 || i == end - 1){
            lock_guard<mutex> lock(top_docs_mutex);
            for(int j = 0;j < buffer_idx; j++){
                if(top_docs.size() < num_top_docs)
                    top_docs.push(buffer[j]);
                else if(buffer[j].first > top_docs.top().first){
                    top_docs.pop();
                    top_docs.push(buffer[j]);
                }
            }
            buffer_idx = 0;
        }
    }
}
// Only use for small values of top_docs_per_thread
void Scorer::rescore_documents(const Dataset &dataset,
                               const vector<float> &weights,
                               int num_threads,
                               int K,
                               const map<int, int> &judgments,
                               vector<int> &top_docs_results)
{
    vector<thread> t;
    mutex top_docs_mutex;
    auto *top_docs = new priority_queue<pair<float, int>>;

    // Fix the last segment to contain everything remaining
    for(int i = 0; i< num_threads;i++){
        t.push_back(
            thread(
                &Scorer::score_docs_priority_queue,
                cref(dataset),
                cref(weights),
                i * dataset.size()/num_threads,
                (i == num_threads - 1)?dataset.size():(i+1) * dataset.size()/num_threads,
                ref(*top_docs),
                ref(top_docs_mutex),
                K,
                ref(judgments)
            )
        );
    }
    for(thread &x: t)
        x.join();

    while(!top_docs->empty()){
        top_docs_results.push_back(top_docs->top().second);
        top_docs->pop();
    }
    delete top_docs;
}

// Todo: Do something about this!
vector<pair<int, float>> Scorer::rescore_all_documents(
        const Dataset &dataset,
        const vector<float> &weights, int num_threads){
    vector<pair<int, float>> top_docs_results;
    for(int i = 0;i<dataset.size(); i++){
        float score = 0.0;
        for(auto feature: dataset.get_sf_sparse_vector(i).features_){
            score += weights[feature.id_] * feature.value_;
        }
        top_docs_results.push_back({i, score});
    }
    sort(top_docs_results.begin(), top_docs_results.end(), [](auto a, auto b) -> bool{ return a.second > b.second;});
    return top_docs_results;
}

