#include <iostream>
#include "featurizer/tfidf.h"
#include "dataset/dataset-memory.h"

using namespace std;
int main() {
    unique_ptr<Featurizer> tfidf = unique_ptr<Featurizer>(new TFIDFFeaturizer());
    string doc1 = "hello dude, What's up?";
    string doc2 = "hello mate, L.O.L dawg!";
    tfidf->fit(doc1);
    tfidf->fit(doc2, true);
    DatasetMemory dataset(move(tfidf));
    dataset.add_doc("doc1", doc1);
    dataset.add_doc("doc2", doc2);
    dataset.write("data.svmlight", DatasetMemory::SVMLIGHT);
    auto spv = dataset.get_features("doc1");
    cout<<spv->features_[0].id_<<endl;
    /* auto sf = tfidf.get_features("Ajdskjsd mate, dawg hello?"); */
    /* for(auto fv: sf.features_){ */
        /* cout<<fv.id_<<" "<<fv.value_<<endl; */
    /* } */
    /* tfidf.write("tfidf.txt"); */
}
