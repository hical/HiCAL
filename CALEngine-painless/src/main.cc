#include <iostream>

#include "dataset/dataset-memory.h"
#include "featurizer/tfidf.h"

using namespace std;
int main() {
    unique_ptr<Featurizer> tfidf =
        unique_ptr<Featurizer>(new TFIDFFeaturizer());
    string doc1 = "hello dude, What's up?";
    string doc2 = "hello mate, L.O.L dawg!";
    tfidf->fit(doc1);
    tfidf->fit(doc2);
    tfidf->finalize();
    DatasetMemory dataset(move(tfidf));
    dataset.add("doc1", doc1);
    dataset.add("doc2", doc2);
    dataset.write("data.svmlight", DatasetMemory::SVMLIGHT);
    auto &item = dataset.get("doc1");
    cout << item.featureVector.features_[0].id_ << endl;
}
