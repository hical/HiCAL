#include <fstream>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>

#include "features.h"
#include "utils/text_utils.h"
using namespace std;

unordered_map<string, features::TermInfo> features::dictionary;

void features::init(const string &fname){
    int df;
    string term;
    int idx = 0;

    ifstream fin(fname);
    while(fin>>df>>term){
        idx++;
        if(df < 2)
            continue;
        dictionary[term] = {idx, df};
    }
}

unordered_map<string, int> features::get_tf(const vector<string> &words){
    unordered_map<string, int> tf_map;
    for(string word: words){
        if(tf_map.find(word) == tf_map.end())
            tf_map[word] = 0;
        tf_map[word]++;
    }
    return tf_map;
}

SfSparseVector features::get_features(const string &text, int N){
    vector<FeatureValuePair> features;
    vector<pair<uint32_t, double>> tmp_features;

    double sum = 0;
    for(pair<string, int> term: get_tf(BMITokenizer().tokenize(text))){
        if(dictionary.find(term.first) != dictionary.end()){
            int id = dictionary[term.first].id;
            int df = dictionary[term.first].df;
            int tf = term.second;
            tmp_features.push_back({id, ((1+log(tf)) * log(N/(float)df))});
            sum += tmp_features.back().second * tmp_features.back().second;
        }
    }
    sum = sqrt(sum);

    for(auto &feature: tmp_features){
        features.push_back({feature.first, (float)(feature.second/sum)});
    }
    sort(features.begin(), features.end(), [](auto &a, auto &b) -> bool{return a.id_ < b.id_;});
    return SfSparseVector("Q", features);
}
