#include <dataset/dataset-memory.h>
#include <featurizer/tfidf.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;

TEST(DatasetMemory, basic) {
    auto tfidf = make_unique<TFIDFFeaturizer>();
    string doc1 = "hello dude, What's up?";
    string doc2 = "hello mate, L.O.L FOO!";
    string doc3 = "bar foo, FOO!";
    tfidf->set_min_df(2);
    tfidf->fit(doc1);
    tfidf->fit(doc2);
    tfidf->fit(doc3);
    tfidf->finalize();
    DatasetMemory dataset(move(tfidf));
    dataset.add("doc1", doc1);
    dataset.add("doc2", doc2);
    dataset.add("doc3", doc3);

    EXPECT_EQ(dataset.size(), 3);
    EXPECT_THROW(dataset.get("doc13"), std::out_of_range);

    auto item = dataset.get("doc1");
    EXPECT_EQ(item.id, "doc1");
    EXPECT_EQ(item.featureVector.features_.size(), 1);

    item = dataset.get("doc2");
    EXPECT_EQ(item.id, "doc2");
    EXPECT_EQ(item.featureVector.features_.size(), 2);

    item = dataset.get("doc3");
    EXPECT_EQ(item.id, "doc3");
    EXPECT_EQ(item.featureVector.features_.size(), 1);

    EXPECT_FLOAT_EQ(item.featureVector.features_[0].value_, 0.0343256052304386);
}

TEST(DatasetMemory, svmlightio) {
    unique_ptr<Featurizer> tfidf =
        unique_ptr<Featurizer>(new TFIDFFeaturizer());
    string doc1 = "hello dude, What's up?";
    string doc2 = "hello mate, L.O.L dawg!";
    tfidf->fit(doc1);
    tfidf->fit(doc2);
    tfidf->finalize();
    tfidf->write("dataset.tfidf");
    DatasetMemory dataset(move(tfidf));
    dataset.add("doc1", doc1);
    dataset.add("doc2", doc2);
    dataset.write("dataset.svmlight", DatasetMemory::SVMLIGHT);

    unique_ptr<Featurizer> tfidf2 =
        unique_ptr<Featurizer>(new TFIDFFeaturizer("dataset.tfidf"));
    DatasetMemory dataset2(move(tfidf), "dataset.svmlight",
                           DatasetMemory::SVMLIGHT);
    EXPECT_EQ(dataset.size(), dataset2.size());
    for (auto item1 : dataset) {
        auto item2 = dataset2.get(item1.id);
        EXPECT_EQ(item1.id, item2.id);
        auto& spv1 = item1.featureVector;
        auto& spv2 = item2.featureVector;
        EXPECT_EQ(spv1.features_.size(), spv2.features_.size());
        for (size_t j = 0; j < spv1.features_.size(); j++) {
            EXPECT_NEAR(spv1.features_[j].value_, spv2.features_[j].value_,
                        1e-6);
        }
    }
}
