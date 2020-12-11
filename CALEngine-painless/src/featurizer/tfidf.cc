#include <featurizer/tfidf.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

using namespace std;

TFIDFFeaturizer::TFIDFFeaturizer(const std::string &filename) {
    ifstream fin(filename);
    string token;
    uint64_t df;
    uint32_t id;
    fin >> total_docs_;
    while (fin >> id >> df >> token) {
        dictionary_[token] = {id, df};
        if (fin.fail()) {
            FATAL("Error parsing featurizer file");
        }
    }
}

void TFIDFFeaturizer::fit(const std::string &text) {
    total_docs_++;
    for (auto token : get_tf(tokenizer_.tokenize(text))) {
        if (dictionary_.find(token.first) == dictionary_.end()) {
            dictionary_[token.first] = {0, 0};
        }
        dictionary_[token.first].df++;
    }
}

void TFIDFFeaturizer::finalize() {
    for (auto it = dictionary_.begin(); it != dictionary_.end();) {
        if (it->second.df < min_df_) {
            dictionary_.erase(it++);
        } else {
            ++it;
        }
    }
    for (auto &dict_item : dictionary_) {
        dict_item.second.id = tokens_.size();
        tokens_.push_back(cref(dict_item.first));
    }
}

void TFIDFFeaturizer::write(const std::string &filename) const {
    ofstream fout(filename);
    fout << total_docs_ << "\n";
    for (auto entry : dictionary_) {
        fout << entry.second.id << " " << entry.second.df << " " << entry.first
             << "\n";
    }
}

SfSparseVector TFIDFFeaturizer::transform(const std::string &text) const {
    vector<FeatureValuePair> features;
    double sum = 0;
    for (auto token : get_tf(tokenizer_.tokenize(text))) {
        auto it = dictionary_.find(token.first);
        if (it != dictionary_.end()) {
            auto tf = token.second;
            double idf = log(total_docs_ / (float)it->second.df);
            double tfidf = (1 + log(tf)) * idf;
            features.push_back({it->second.id, float(tfidf)});
            sum += tfidf * tfidf;
        }
    }

    sum = sqrt(sum);
    for (auto &feature : features) {
        feature.value_ /= float(max(sum, max_norm_));
    }

    sort(features.begin(), features.end(),
         [](auto &a, auto &b) { return a.id_ < b.id_; });
    return SfSparseVector(features);
}
