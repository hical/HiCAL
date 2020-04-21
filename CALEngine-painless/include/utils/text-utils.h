#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

// Filter declarations start here
class Filter {
   public:
    virtual bool filter(const std::string &token) const = 0;
};

class AlphaFilter : public Filter {
   public:
    bool filter(const std::string &token) const override;
};

class MinLengthFilter : public Filter {
    size_t min_length;

   public:
    MinLengthFilter(size_t _min_length) : min_length(_min_length){};
    bool filter(const std::string &token) const override;
};

// Transformer declarations start here
class Transform {
   public:
    virtual std::string transform(const std::string &token) const = 0;
};

class PorterTransform : public Transform {
   public:
    std::string transform(const std::string &token) const override;
};

class LowerTransform : public Transform {
   public:
    std::string transform(const std::string &token) const override;
};

// Tokenizer declarations start here
class Tokenizer {
   public:
    virtual std::vector<std::string> tokenize(
        const std::string &text) const = 0;
};

class BMITokenizer : public Tokenizer {
    AlphaFilter alpha_filter = AlphaFilter();
    MinLengthFilter min_length_filter = MinLengthFilter(2);
    PorterTransform porter_transform = PorterTransform();
    LowerTransform lower_transform = LowerTransform();

   public:
    std::vector<std::string> tokenize(const std::string &text) const override;
};

std::unordered_map<std::string, uint32_t> get_tf(
    const std::vector<std::string> &words);
#endif  // TEXT_UTILS_H
