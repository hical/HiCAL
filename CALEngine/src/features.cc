#include<iostream>
#include<fstream>
#include<string>
#include<cstring>
#include<vector>
#include<unordered_map>
#include<cmath>
#include<algorithm>
#include "porter.c"
#include "sofiaml/sf-sparse-vector.h"
using namespace std;

struct TermInfo{
    int id;
    int df;
};

unordered_map<string, TermInfo> dictionary;

bool is_valid(char ch){
    return isalpha(ch) || isdigit(ch);
}

vector<string> tokenize(string &text){
    vector<string> words;
    int st = 0;
    while(st < (int)text.length()){
        int end = 0;
        while(isalpha(text[st+end])){
            end++;
        }
        if(end > 0){
            words.push_back(text.substr(st, end));
        }
        st += end + 1;
    }
    return words;
}

vector<string> get_stemmed_words(const string &str){
    char temp_str[str.length()];
    strcpy(temp_str, str.c_str());
    int st = 0, end = 0;
    string stemmed_text;

    while(temp_str[st]){
        if(!isalpha(temp_str[st])){
            stemmed_text.push_back(temp_str[st++]);
        }else{
            end = 0;
            while(isalpha(temp_str[st+end])){
                temp_str[st+end] = tolower(temp_str[st+end]);
                end++;
            }
            end--;
            int new_end = stem(temp_str, st, st + end);
            int final_st = st + end + 1;
            while(st <= new_end){
                stemmed_text.push_back(temp_str[st++]);
            }
            st = final_st;
        }
    }
    return tokenize(stemmed_text);
}

void init(string fname){
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

unordered_map<string, int> get_tf(vector<string> words){
    unordered_map<string, int> tf_map;
    for(string word: words){
        if(tf_map.find(word) == tf_map.end())
            tf_map[word] = 0;
        tf_map[word]++;
    }
    return tf_map;
}

SfSparseVector get_features(string &text, int N){
    vector<FeatureValuePair> features;
    vector<pair<int, double>> tmp_features;

    double sum = 0;
    for(pair<string, int> term: get_tf(get_stemmed_words(text))){
        if(dictionary.find(term.first) != dictionary.end()){
            int id = dictionary[term.first].id;
            int df = dictionary[term.first].df;
            int tf = term.second;
            tmp_features.push_back({id, ((1+log(tf)) * log(N/(float)df))});
            sum += tmp_features.back().second * tmp_features.back().second;
        }
    }
    sum = sqrt(sum);

    for(pair<int, double> feature: tmp_features){
        features.push_back({feature.first, (float)(feature.second/sum)});
    }
    sort(features.begin(), features.end(), [](auto &a, auto &b) -> bool{return a.id_ < b.id_;});
    return SfSparseVector("Q", features);
}

/* int main(){ */
/*     init("/home/nghelani/CAL/Corpus/athome1.df"); */
/*     string str = "School and Preschool Funding"; */
/*     get_features(str, 290099); */
/* } */
