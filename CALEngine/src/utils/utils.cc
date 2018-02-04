#include "utils.h"

void msg(const char *message){
    fprintf(stderr, "%s", message);
    fprintf(stderr, "\n");
}

void fail(const char *message, int e){
    msg(message);
    exit(e);
}

void msg(const std::string &str){
    msg(str.c_str());
}

void fail(const std::string &str, int e){
    fail(str.c_str(), e);
}
