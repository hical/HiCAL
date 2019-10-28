#ifndef TFIDF_H
#define TFIDF_H

#include "featurizer/featurizer.h"
#include "utils/text-utils.h"
#include <unordered_map>

struct TermInfo {
  uint32_t id;
  uint64_t df;
};

class TFIDFFeaturizer : public Featurizer {
  std::unordered_map<std::string, TermInfo> dictionary_;
  BMITokenizer tokenizer_ = BMITokenizer();
  size_t total_docs_ = 0;
  double max_norm_ = 20;
  uint64_t min_df_ = 0;

public:
  TFIDFFeaturizer();

  TFIDFFeaturizer(const std::string &filename);

  virtual void fit(const std::string &text);

  virtual void write(const std::string &filename);
  
  virtual void finalize();

  virtual SfSparseVector get_features(const std::string &text);

  virtual void set_min_df(uint64_t min_df) {
      min_df_ = min_df;
  }
};

#endif // TFIDF_H
