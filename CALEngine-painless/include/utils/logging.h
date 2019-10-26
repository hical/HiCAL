#ifndef LOGGING_H
#define LOGGING_H

#include <cstdio>
#include <cstdlib>
#include <string>

inline void FATAL(const char *msg) {
    fprintf(stderr, "[FATAL] %s\n", msg);
    exit(1);
}

inline void INFO(const char *msg) {
    fprintf(stderr, "[INFO] %s\n", msg);
}

inline void WARN(const char *msg) {
    fprintf(stderr, "[WARN] %s\n", msg);
}

inline void FATAL(const std::string &str) {
    FATAL(str.c_str());
}

inline void INFO(const std::string &str) {
    INFO(str.c_str());
}

inline void WARN(const std::string &str) {
    WARN(str.c_str());
}

#endif // LOGGING_H
