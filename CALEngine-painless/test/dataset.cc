#include "featurizer/tfidf.h"
#include "dataset/dataset-memory.h"
#include "gtest/gtest.h"

using namespace std;

TEST(DatasetMemory, basic) {
    unique_ptr<Featurizer> tfidf = unique_ptr<Featurizer>(new TFIDFFeaturizer());
    string doc1 = "hello dude, What's up?";
    string doc2 = "hello mate, L.O.L FOO!";
    string doc3 = "bar foo, FOO!";
    tfidf->fit(doc1);
    tfidf->fit(doc2);
    tfidf->fit(doc3);
    tfidf->finalize();
    DatasetMemory dataset(move(tfidf));
    dataset.add_doc("doc1", doc1);
    dataset.add_doc("doc2", doc2);
    dataset.add_doc("doc3", doc3);

    EXPECT_EQ(dataset.size(), 3);
    auto spv = dataset.get_features("doc13");
    EXPECT_EQ(spv, nullptr);

    spv = dataset.get_features("doc1");
    EXPECT_EQ(spv->doc_id, "doc1");
    EXPECT_EQ(spv->features_.size(), 2);

    spv = dataset.get_features("doc2");
    EXPECT_EQ(spv->doc_id, "doc2");
    EXPECT_EQ(spv->features_.size(), 3);

    spv = dataset.get_features("doc3");
    EXPECT_EQ(spv->doc_id, "doc3");
    EXPECT_EQ(spv->features_.size(), 2);

    EXPECT_EQ(spv->features_[0].id_, 0);
    EXPECT_EQ(spv->features_[0].value_, 1);
    EXPECT_EQ(spv->features_[1].id_, 3);
    EXPECT_EQ(spv->features_[1].value_, 0);
    EXPECT_EQ(spv->features_[2].id_, 5);
    EXPECT_NEAR(spv->features_[2].value_, 0.0346574, 1e-6);
}

TEST(DatasetMemory, svmlightio) {
    unique_ptr<Featurizer> tfidf = unique_ptr<Featurizer>(new TFIDFFeaturizer());
    string doc1 = "hello dude, What's up?";
    string doc2 = "hello mate, L.O.L dawg!";
    tfidf->fit(doc1);
    tfidf->fit(doc2);
    tfidf->finalize();
    tfidf->write("dataset.tfidf");
    DatasetMemory dataset(move(tfidf));
    dataset.add_doc("doc1", doc1);
    dataset.add_doc("doc2", doc2);
    dataset.write("dataset.svmlight", DatasetMemory::SVMLIGHT);

    unique_ptr<Featurizer> tfidf2 = unique_ptr<Featurizer>(new TFIDFFeaturizer("dataset.tfidf"));
    DatasetMemory dataset2(move(tfidf), "dataset.svmlight", DatasetMemory::SVMLIGHT);
    EXPECT_EQ(dataset.size(), dataset2.size());
    for(size_t i = 0; i < dataset.size(); i++){
        auto spv1 = dataset.get_features(i);
        auto spv2 = dataset2.get_features(i);
        EXPECT_EQ(spv1->doc_id, spv2->doc_id);
        EXPECT_EQ(spv1->features_.size(), spv2->features_.size());
        for(size_t j = 0; j < spv1->features_.size(); j++){
            EXPECT_EQ(spv1->features_[j].id_, spv2->features_[j].id_);
            EXPECT_NEAR(spv1->features_[j].value_, spv2->features_[j].value_, 1e-6);
        }
    }
}
