#ifndef LOGGING_H
#define LOGGING_H

#include <cstdio>
#include <cstdlib>

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

#endif // LOGGING_H
