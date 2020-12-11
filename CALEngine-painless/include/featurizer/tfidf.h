#ifndef TFIDF_H
#define TFIDF_H

#include <featurizer/featurizer.h>
#include <utils/text-utils.h>

#include <functional>
#include <unordered_map>

struct TermInfo {
    uint32_t id;
    uint64_t df;
};

class TFIDFFeaturizer : public Featurizer {
    const BMITokenizer tokenizer_ = BMITokenizer();
    size_t total_docs_ = 0;
    double max_norm_ = 20;
    uint64_t min_df_ = 0;

   public:
    std::unordered_map<std::string, TermInfo> dictionary_;
    std::vector<std::reference_wrapper<const std::string>> tokens_;
    TFIDFFeaturizer() = default;

    TFIDFFeaturizer(const std::string &filename);

    virtual void fit(const std::string &text);

    virtual void write(const std::string &filename) const;

    virtual void finalize();

    virtual SfSparseVector transform(const std::string &text) const;

    void set_min_df(uint64_t min_df) { min_df_ = min_df; }
    void set_max_norm(uint64_t max_norm) { max_norm_ = max_norm; }
};

#endif  // TFIDF_H
