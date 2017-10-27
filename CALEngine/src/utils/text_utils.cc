#include <iostream>
#include <algorithm>
#include "text_utils.h"
#include "porter.c"

using namespace std;
bool AlphaFilter::filter(const std::string &token) {
    if(token.length() < 2)
        return false;
    for(char c: token){
        if(!isalpha(c))
            return false;
    }
    return true;
}

bool MinLengthFilter::filter(const string &token) {
    return token.length() >= min_length;
}

std::string PorterTransform::transform(const std::string &token) {
    char temp_str[token.length()+1];
    strcpy(temp_str, token.c_str());

    int end = stem(temp_str, 0, (int)token.length()-1);
    return std::string(temp_str, end+1);
}

std::string LowerTransform::transform(const std::string &token) {
    string transformed_token = token;
    std::transform(transformed_token.begin(), transformed_token.end(), transformed_token.begin(), ::tolower);
    return transformed_token;
}

std::vector<std::string> BMITokenizer::tokenize(const std::string &text) {
    vector<string> tokens;
    int st = 0;
    while(st < (int)text.length()){
        int end = 0;
        while(isalnum(text[st+end])){
            end++;
        }
        if(end > 0){
            auto token = text.substr(st, end);
            if(alpha_filter.filter(token)){
                token = porter_transform.transform(lower_transform.transform(token));
                if(min_length_filter.filter(token))
                    tokens.push_back(token);
            }
        }
        st += end + 1;
    }
    return tokens;
}
