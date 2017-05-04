#include<iostream>
#include<vector>
#include<fstream>
#include<cstring>
#include<thread>
#include<algorithm>
#include<unordered_map>
#include "scorer.h"

using namespace std;

vector<SfSparseVector> doc_features;
unordered_map<string, int> doc_ids_inv_map;

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

vector<float> parse_model_file(string fname){
    ifstream model_file(fname);
    vector<float> weights;
    float weight;
    while(model_file >> weight){
        weights.push_back(weight);
    }
    model_file.close();
    return weights;
}

void score_docs(const vector<float> &weights, 
        int st,
        int end, 
        pair<float, int> *top_docs,
        int num_top_docs,
        const vector<int> &judgments)
{
    auto iterator = lower_bound(judgments.begin(), judgments.end(), st);
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

void rescore_documents(const vector<float> &weights,
        int num_threads, 
        int top_docs_per_thread,
        const vector<int> &judgments,
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
                score_docs,
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

void set_doc_features(string fname){
    doc_features = parse_doc_features(fname);
    for(size_t i = 0; i < doc_features.size(); i++){
        doc_ids_inv_map[doc_features[i].doc_id] = i;
    }
}
