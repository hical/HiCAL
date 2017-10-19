#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <unordered_map>
#include "scorer.h"

using namespace std;

void Scorer::score_docs(
        const Dataset &dataset,
        const vector<float> &weights,
        int st,
        int end,
        pair<float, int> *top_docs,
        int num_top_docs,
        const set<int> &judgments) {
    auto iterator = judgments.lower_bound(st);
    for(int i = st;i<end; i++){
        while(iterator != judgments.end() && *iterator < i)
            iterator++;
        if(iterator != judgments.end() && *iterator == i)
            continue;

        float score = 0;
        for(auto feature: dataset.get_sf_sparse_vector(i).features_){
            score += weights[feature.id_] * feature.value_;
        }

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

// Only use for small values of top_docs_per_thread
void Scorer::rescore_documents(const Dataset &dataset,
        const vector<float> &weights,
        int num_threads, 
        int top_docs_per_thread,
        const set<int> &judgments,
        vector<int> &top_docs_results)
{
    vector<thread> t;

    pair<float, int> top_docs[top_docs_per_thread * num_threads];
    for(int i = 0;i<top_docs_per_thread * num_threads; i++)
        top_docs[i] = {-1e9, -1};
    
    // Fix the last segment to contain everything remaining
    for(int i = 0; i< num_threads;i++){
        t.push_back(
            thread(
                &Scorer::score_docs,
                cref(dataset),
                cref(weights),
                i * dataset.size()/num_threads,
                (i == num_threads - 1)?dataset.size():(i+1) * dataset.size()/num_threads,
                top_docs + top_docs_per_thread * i,
                top_docs_per_thread,
                ref(judgments)
            )
        );
    }
    for(thread &x: t)
        x.join();

    sort(top_docs, top_docs + top_docs_per_thread * num_threads, greater<pair<float, int>>());
    for(int i = 0;i<top_docs_per_thread; i++){
        top_docs_results.push_back(top_docs[i].second);
    }
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

vector<pair<int, float>> Scorer::get_top_terms(const vector<float> &weights, const SfSparseVector &document, int num_top_terms){
    vector<pair<int, float>> feature_weights;
    for(auto feature: document.features_){
        feature_weights.push_back({feature.id_, weights[feature.id_]});
    }
    sort(feature_weights.begin(), feature_weights.end(),
            [](const pair<int, float> &a, const pair<int, float> &b)->bool{return a.second > b.second;});
    return vector<pair<int, float>>(feature_weights.begin(),feature_weights.begin() + min(num_top_terms, (int)feature_weights.size()));
}