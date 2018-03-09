#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <chrono>

#define TIMER_BEGIN(key) \
    auto start##key = std::chrono::steady_clock::now();

#define TIMER_END(key) \
    std::cerr<<"("<< #key <<"): "<<(std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - start##key)).count()<<"ms\n";

void msg(const char *message);
void fail(const char *message, int e);
void msg(const std::string &str);
void fail(const std::string &str, int e);
#endif // UTILS_H
