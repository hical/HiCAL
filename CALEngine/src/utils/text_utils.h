#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <string>
#include <vector>

namespace text_utils {
    std::vector<std::string> tokenize(const std::string &text);
    std::vector<std::string> get_stemmed_words(const std::string &str);
}
#endif // TEXT_UTILS_H
