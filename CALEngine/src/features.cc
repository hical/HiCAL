#include <fstream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "features.h"
#include "utils/text_utils.h"
using namespace std;

unordered_map<string, int> features::get_tf(const vector<string> &words){
    unordered_map<string, int> tf_map;
    for(string word: words){
        if(tf_map.find(word) == tf_map.end())
            tf_map[word] = 0;
        tf_map[word]++;
    }
    return tf_map;
}

SfSparseVector features::get_features(const string &text, const Dataset &dataset, double max_norm){
    vector<FeatureValuePair> features;
    vector<pair<uint32_t, double>> tmp_features;

    double sum = 0;
    auto &dictionary = dataset.get_dictionary();
    for(pair<string, int> term: get_tf(BMITokenizer().tokenize(text))){
        auto it = dictionary.find(term.first);
        if(it != dictionary.end()){
            int id = it->second.id;
            int df = it->second.df;
            int tf = term.second;
            tmp_features.push_back({id, ((1+log(tf)) * log(dataset.size()/(float)df))});
            sum += tmp_features.back().second * tmp_features.back().second;
        }
    }
    sum = sqrt(sum);

    for(auto &feature: tmp_features){
        features.push_back({feature.first, (float)(feature.second/max(max_norm, sum))});
    }
    sort(features.begin(), features.end(), [](auto &a, auto &b) -> bool{return a.id_ < b.id_;});
    return SfSparseVector("Q", features);
}
