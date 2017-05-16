#include <vector>
#include <fstream>
#include <cstring>
#include <thread>
#include <algorithm>
#include <unordered_map>
#include "scorer.h"

using namespace std;

vector<SfSparseVector> parse_doc_features(string fname){
    vector<SfSparseVector> sparse_feature_vectors;
    ifstream doc_features_file(fname);

    string line;
    while(getline(doc_features_file, line)){
        const char* line_chr = line.c_str();
        int len = strlen(line_chr);
        while(line_chr[len-1] == '\n')
            len--;
        const char* moving_chr = line_chr;

        string doc_id = "";
        for(int i = 0;moving_chr[i]!=' ';i++)
            doc_id.push_back(moving_chr[i]);

        vector<FeatureValuePair> features;
        while(moving_chr < line_chr + len){
            moving_chr = strchr(moving_chr, ' ');
            if(moving_chr == NULL)
                break;
            moving_chr++;

            int feature_id = atoi(moving_chr);
            moving_chr = strchr(moving_chr, ':') + 1;
            float feature_weight = atof(moving_chr);

            features.push_back({feature_id, feature_weight});
        }
        sparse_feature_vectors.push_back(SfSparseVector(doc_id, features));
    }
    doc_features_file.close();
    return sparse_feature_vectors;
}

void Scorer::score_docs(const vector<float> &weights, 
        int st,
        int end, 
        pair<float, int> *top_docs,
        int num_top_docs,
        const set<int> &judgments)
{
    auto iterator = judgments.lower_bound(st);
    for(int i = st;i<end; i++){
        while(iterator != judgments.end() && *iterator < i)
            iterator++;
        if(iterator != judgments.end() && *iterator == i)
            continue;
        float score = 0;
        for(auto feature: doc_features[i].features_){
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

void Scorer::rescore_documents(const vector<float> &weights,
        int num_threads, 
        int top_docs_per_thread,
        const set<int> &judgments,
        vector<int> &top_docs_results)
{
    vector<thread> t;

    pair<float, int> top_docs[top_docs_per_thread * num_threads];
    for(int i = 0;i<top_docs_per_thread * num_threads; i++)
        top_docs[i] = {-1, -1};
    
    // Fix the last segment to contain everything remaining
    for(int i = 0; i< num_threads;i++){
        t.push_back(
            thread(
                &Scorer::score_docs,
                this,
                ref(weights),
                i * doc_features.size()/num_threads,
                (i == num_threads - 1)?doc_features.size():(i+1) * doc_features.size()/num_threads,
                top_docs + top_docs_per_thread * i,
                top_docs_per_thread,
                ref(judgments)
            )
        );
    }
    for(thread &x: t)
        x.join();

    sort(top_docs, top_docs + top_docs_per_thread * num_threads, greater<pair<float, int>>());
    for(int i = 0;i<top_docs_per_thread; i++)
        top_docs_results.push_back(top_docs[i].second);
}

vector<pair<int, float>> Scorer::get_top_terms(const vector<float> &weights, string doc_id, int num_top_terms){
    vector<pair<int, float>> feature_weights;
    int doc_idx = doc_ids_inv_map[doc_id];
    for(auto feature: doc_features[doc_idx].features_){
        feature_weights.push_back({feature.id_, weights[feature.id_]});
    }
    sort(feature_weights.begin(), feature_weights.end(),
            [](const pair<int, float> &a, const pair<int, float> &b)->bool{return a.second > b.second;});
    return vector<pair<int, float>>(feature_weights.begin(),feature_weights.begin() + min(num_top_terms, (int)feature_weights.size()));
}

Scorer::Scorer(string fname){
    doc_features = parse_doc_features(fname);
    for(size_t i = 0; i < doc_features.size(); i++){
        doc_ids_inv_map[doc_features[i].doc_id] = i;
    }

    dimensionality = 0;
    for(auto &feature_vec: doc_features){
        for(auto feature: feature_vec.features_)
            dimensionality = max(dimensionality, feature.id_);
    }
    dimensionality++;
}
