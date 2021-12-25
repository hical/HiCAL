#ifndef CAL_PROCESS_H
#define CAL_PROCESS_H

#include <string>
#include <utility>
#include <vector>

namespace cal {

enum class Relevance {

};

class BaseProcess {
   public:
    virtual void judgeDoc(const std::string &docId, Relevance rel) = 0;
    virtual void judgeText(const std::string &text, Relevance rel) = 0;
    virtual std::vector<std::string> getJudgmentQueue(size_t maxSize) = 0;
    virtual std::vector<std::pair<std::string, float>> getRanklist(size_t) = 0;
    virtual std::vector<std::pair<std::string, Relevance>>
};

}  // namespace cal

#endif  // CAL_PROCESS_H
