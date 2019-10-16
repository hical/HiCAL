#include "featurizer/tfidf.h"
#include <algorithm>
#include <cmath>
#include <fstream>

using namespace std;

TFIDFFeaturizer::TFIDFFeaturizer() {}

TFIDFFeaturizer::TFIDFFeaturizer(const std::string &filename) {
  ifstream fin(filename);
  string token;
  uint64_t df;
  uint32_t id;
  while (fin >> id >> df >> token) {
    dictionary_[token] = {id, df};
    if (fin.fail()) {
      FATAL("Error parsing featurizer file");
    }
  }

  Featurizer::finalize();
}

void TFIDFFeaturizer::fit(const std::string &text) {
  total_docs_++;
  for (auto token : get_tf(tokenizer_.tokenize(text))) {
    if (dictionary_.find(token.first) == dictionary_.end()) {
      uint32_t new_id = dictionary_.size() + 1;
      dictionary_[token.first] = {new_id, 0};
    }
    dictionary_[token.first].df++;
  }
}

void TFIDFFeaturizer::write(const std::string &filename) {
  ofstream fout(filename);
  for (auto entry : dictionary_) {
    fout << entry.second.id << " " << entry.second.df << " " << entry.first
         << "\n";
  }
}

SfSparseVector TFIDFFeaturizer::get_features(const std::string &text) {
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
       [](auto &a, auto &b) -> bool { return a.id_ < b.id_; });
  return SfSparseVector("tfidf", features);
}
