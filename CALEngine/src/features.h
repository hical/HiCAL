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

}
#endif // FEATURES_H
