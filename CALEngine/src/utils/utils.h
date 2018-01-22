#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <chrono>
#define TIMER_BEGIN {\
    auto start = std::chrono::steady_clock::now();

#define TIMER_END(log) \
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - start);\
    std::cerr<<"("<<log<<"): "<<duration.count()<<"ms";\
}

void msg(const char *message);
void fail(const char *message, int e);
void msg(const std::string &str);
void fail(const std::string &str, int e);
#endif // UTILS_H
