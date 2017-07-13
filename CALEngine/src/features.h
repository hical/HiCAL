#ifndef FEATURES_H
#define FEATURES_H

#include <string>
#include <vector>
#include <unordered_map>
#include "sofiaml/sf-sparse-vector.h"

namespace features{
    const char DELIM_CHAR = '\n';
    struct TermInfo{
        int id;
        int df;
    };

    extern std::unordered_map<string, TermInfo> dictionary;

    inline bool is_valid(char ch){
        return isalpha(ch) || isdigit(ch);
    }

    std::vector<std::string> tokenize(const std::string &text);
    std::vector<std::string> get_stemmed_words(const std::string &str);

    void init(const std::string &fname);

    std::unordered_map<std::string, int> get_tf(const vector<std::string> &words);

    // Extract features from given text
    SfSparseVector get_features(const std::string &text, int N);

    // Converts svmlight feature format to binary format
    // First 4 bytes (uint32_t) is the number of records in the file. Records start after this.
    // Bytes until DELIM_CHAR denote the id of the record. Next 4 bytes (uint32_t) tell the number of
    // feature-weight pairs in this record. Feature-weight pairs start after this. Each pair is 4 bytes of
    // feature_id (uint32_t) followed by 4 bytes of feature_weight (float)
    void svmlight_to_bin(string input_fname, string output_fname);
    void bin_to_svmlight(string input_fname, string output_fname);
    vector<SfSparseVector> parse_svmlight_features(string fname);
    vector<SfSparseVector> parse_bin_features(string fname);
}
#endif // FEATURES_H
