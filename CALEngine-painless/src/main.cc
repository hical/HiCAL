#include <iostream>
#include "featurizer/tfidf.h"

using namespace std;
int main() {
    TFIDFFeaturizer tfidf;
    tfidf.fit("hello dude, What's up?");
    tfidf.fit("hello mate, L.O.L dawg!", true);
    auto sf = tfidf.get_features("Ajdskjsd mate, dawg hello?");
    for(auto fv: sf.features_){
        cout<<fv.id_<<" "<<fv.value_<<endl;
    }
    tfidf.write("tfidf.txt");
}
