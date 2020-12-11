#include "utils/text-utils.h"

#include <algorithm>
#include <iostream>
#include <utils/porter.c>

using namespace std;

bool AlphaFilter::filter(const std::string &token) const {
    for (char c : token) {
        if (!isalpha(c)) return false;
    }
    return true;
}

bool MinLengthFilter::filter(const string &token) const {
    return token.length() >= min_length;
}

std::string PorterTransform::transform(const std::string &token) const {
    char temp_str[token.length() + 1];
    strcpy(temp_str, token.c_str());

    int end = stem(temp_str, 0, (int)token.length() - 1);
    return std::string(temp_str, end + 1);
}

std::string LowerTransform::transform(const std::string &token) const {
    string transformed_token = token;
    std::transform(transformed_token.begin(), transformed_token.end(),
                   transformed_token.begin(), ::tolower);
    return transformed_token;
}

std::vector<std::string> BMITokenizer::tokenize(const std::string &text) const {
    vector<string> tokens;
    int st = 0;
    while (st < (int)text.length()) {
        int end = 0;
        while (isalnum(text[st + end])) {
            end++;
        }
        if (end > 0) {
            auto token = text.substr(st, end);
            if (alpha_filter.filter(token)) {
                token = porter_transform.transform(
                    lower_transform.transform(token));
                if (min_length_filter.filter(token)) tokens.push_back(token);
            }
        }
        st += end + 1;
    }
    return tokens;
}

unordered_map<string, uint32_t> get_tf(const vector<string> &words) {
    unordered_map<string, uint32_t> tf_map;
    for (string word : words) {
        if (tf_map.find(word) == tf_map.end()) tf_map[word] = 0;
        tf_map[word]++;
    }
    return tf_map;
}
