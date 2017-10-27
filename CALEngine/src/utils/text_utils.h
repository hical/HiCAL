#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <string>
#include <vector>
#include <functional>
#include <cstring>


/*
 * Intentionally not optimized for sake of maintenance and flexibility
 * Todo: use char* for internal filters and transforms
 */


// Filter declarations start here
class Filter {
public:
    virtual bool filter(const std::string &token) = 0;
};

class AlphaFilter:public Filter {
public:
    bool filter(const std::string &token) override;
};

class MinLengthFilter:public Filter {
    size_t min_length;
public:
    MinLengthFilter(size_t _min_length):min_length(_min_length){};
    bool filter(const std::string &token) override;
};


// Transformer declarations start here
class Transform {
public:
    virtual std::string transform(const std::string &token) = 0;
};

class PorterTransform:public Transform {
public:
    std::string transform(const std::string &token) override;
};

class LowerTransform:public Transform {
public:
    std::string transform(const std::string &token) override;
};


// Tokenizer declarations start here
class Tokenizer {
public:
    virtual std::vector<std::string> tokenize(const std::string &text) = 0;
};

class BMITokenizer:public Tokenizer {
    AlphaFilter alpha_filter = AlphaFilter();
    MinLengthFilter min_length_filter = MinLengthFilter(2);
    PorterTransform porter_transform = PorterTransform();
    LowerTransform lower_transform = LowerTransform();
public:
    std::vector<std::string> tokenize(const std::string &text) override;
};
#endif // TEXT_UTILS_H
