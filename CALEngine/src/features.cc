#include <fstream>
#include<iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include "porter.c"

#include "features.h"
using namespace std;

unordered_map<string, features::TermInfo> features::dictionary;

vector<string> features::tokenize(const string &text){
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

vector<string> features::get_stemmed_words(const string &str){
    char temp_str[str.length()+1];
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

    for(auto &feature: tmp_features){
        features.push_back({feature.first, (float)(feature.second/sum)});
    }
    sort(features.begin(), features.end(), [](auto &a, auto &b) -> bool{return a.id_ < b.id_;});
    return SfSparseVector("Q", features);
}

void features::svmlight_to_bin(string input_fname, string output_fname){
    ifstream input_file(input_fname);
    FILE *output_file = fopen(output_fname.c_str(), "wb");
    string line;

    // Leave space for header
    fseeko(output_file, sizeof(uint32_t), SEEK_SET);

    uint32_t num_records = 0;
    uint32_t dimen = 0;
    while(getline(input_file, line)){
        dimen++;
        const char* line_chr = line.c_str();
        int len = line.size();
        while(line_chr[len-1] == '\n')
            len--;
        const char* moving_chr = line_chr;

        int i = 0;
        for(i = 0;moving_chr[i]!=' ';i++);
        fwrite(moving_chr, i, 1, output_file);
        fputc(DELIM_CHAR, output_file);
        off_t record_len_offset = ftello(output_file);
        fseeko(output_file, sizeof(uint32_t), SEEK_CUR);

        uint32_t num_pairs = 0;
        vector<FeatureValuePair> features;
        while(moving_chr < line_chr + len){
            moving_chr = strchr(moving_chr, ' ');
            if(moving_chr == NULL)
                break;
            moving_chr++;

            uint32_t feature_id = atoi(moving_chr);
            moving_chr = strchr(moving_chr, ':') + 1;
            float feature_weight = atof(moving_chr);

            fwrite(&feature_id, sizeof(uint32_t), 1, output_file);
            fwrite(&feature_weight, sizeof(float), 1, output_file);
            num_pairs++;
        }

        off_t backup_offset = ftello(output_file);
        fseeko(output_file, record_len_offset, SEEK_SET);
        fwrite(&num_pairs, sizeof(uint32_t), 1, output_file);
        fseeko(output_file, backup_offset, SEEK_SET);
        num_records++;
    }
    input_file.close();
    // Write header
    fseeko(output_file, 0, SEEK_SET);
    fwrite(&num_records, sizeof(uint32_t), 1, output_file);
    fclose(output_file);
}

void features::bin_to_svmlight(string input_fname, string output_fname){
    vector<SfSparseVector> sparse_feature_vectors = features::parse_bin_features(input_fname);
    FILE *output_file = fopen(output_fname.c_str(), "wb");
    for(auto &spv: sparse_feature_vectors){
        fprintf(output_file, "%s", spv.doc_id.c_str());
        for(auto &fpv: spv.features_){
            if(fpv.id_ != 0)
                fprintf(output_file, " %d:%.8f", fpv.id_, fpv.value_);
        }
        fprintf(output_file, "\n");
    }
    fclose(output_file);
}

vector<SfSparseVector> features::parse_svmlight_features(string fname){
    vector<SfSparseVector> sparse_feature_vectors;
    ifstream doc_features_file(fname);

    string line;
    while(getline(doc_features_file, line)){
        const char* line_chr = line.c_str();
        int len = line.size();
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

            uint32_t feature_id = atoi(moving_chr);
            moving_chr = strchr(moving_chr, ':') + 1;
            float feature_weight = atof(moving_chr);

            features.push_back({feature_id, feature_weight});
        }
        sparse_feature_vectors.push_back(SfSparseVector(doc_id, features));
    }
    doc_features_file.close();
    return sparse_feature_vectors;
}

vector<SfSparseVector> features::parse_bin_features(string fname){
    vector<SfSparseVector> sparse_feature_vectors;
    FILE *input_file = fopen(fname.c_str(), "rb");
    uint32_t num_records = 0;
    fread(&num_records, sizeof(num_records), 1, input_file);

    for(uint32_t i = 0;i<num_records; i++){
        string doc_id;
        char c = fgetc(input_file);
        while(c != DELIM_CHAR){
            doc_id.push_back(c);
            c = fgetc(input_file);
        }
        uint32_t num_pairs = 0;
        fread(&num_pairs, sizeof(num_pairs), 1, input_file);

        vector<FeatureValuePair> features(num_pairs);
        for(auto &fvp: features){
            uint32_t x;
            fread(&x, sizeof(uint32_t), 1, input_file);
            fvp.id_ = x;
            fread(&fvp.value_, sizeof(float), 1, input_file);
        }
        sparse_feature_vectors.push_back(SfSparseVector(doc_id, features));
    }

    fclose(input_file);
    return sparse_feature_vectors;
}
