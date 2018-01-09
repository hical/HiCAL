#include <string>
#include <chrono>
#define TIMER_BEGIN {\
    auto start = std::chrono::steady_clock::now();

#define TIMER_END(log) \
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - start);\
    std::cerr<<"("<<log<<"): "<<duration.count()<<"ms";\
}
