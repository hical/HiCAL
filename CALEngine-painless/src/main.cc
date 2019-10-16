#include <iostream>
#include "featurizer/tfidf.h"

using namespace std;
int main() {
    TFIDFFeaturizer tfidf;
    tfidf.fit("hello dude, What's up?");
    tfidf.fit("hello mate, L.O.L dawg!");
    tfidf.finalize();
    tfidf.write("tfidf.txt");
}
