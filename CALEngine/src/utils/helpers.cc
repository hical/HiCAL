#include <cstdio>
#include <cstdlib>

void msg(const char *message){
    fprintf(stderr, "%s", message);
    fprintf(stderr, "\n");
}

void fail(const char *message, int e){
    msg(message);
    exit(e);
}
