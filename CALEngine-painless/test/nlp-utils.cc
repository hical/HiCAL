#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "featurizer/tfidf.h"
#include "utils/sf-sparse-vector.h"
#include "utils/text-utils.h"

using namespace testing;

class TFIDFTest : public Test {
   protected:
    TFIDFFeaturizer tfidf;
    void SetUp() override {
        tfidf.set_min_df(2);
        tfidf.fit("hello dude, What's up?");
        tfidf.fit("hello mate, L.O.L FOO!");
        tfidf.fit("bar foo, FOO!");
        tfidf.finalize();
    }
};

TEST_F(TFIDFTest, basic) {
    auto sf = tfidf.transform("Ajdskjsd mate, FOO hello?");
    EXPECT_EQ(sf.features_.size(), 2);

    EXPECT_THAT(
        sf.features_,
        Contains(AllOf(
            Field(&FeatureValuePair::id_, Eq(tfidf.dictionary_.at("foo").id)),
            Field(&FeatureValuePair::value_, FloatEq(0.020273255184292793)))));

    EXPECT_THAT(
        sf.features_,
        Contains(AllOf(
            Field(&FeatureValuePair::id_, Eq(tfidf.dictionary_.at("hello").id)),
            Field(&FeatureValuePair::value_, FloatEq(0.020273255184292793)))));
}

TEST_F(TFIDFTest, io) {
    tfidf.write("tfidf.txt");
    TFIDFFeaturizer tfidf2("tfidf.txt");
    auto sf1 = tfidf.transform("Ajdskjsd mate, dawg hello?");
    auto sf2 = tfidf2.transform("Ajdskjsd mate, dawg hello?");
    EXPECT_EQ(sf1.features_.size(), sf2.features_.size());
    for (size_t i = 0; i < sf1.features_.size(); i++) {
        EXPECT_EQ(sf1.features_[i].id_, sf2.features_[i].id_);
        EXPECT_EQ(sf1.features_[i].value_, sf2.features_[i].value_);
    }
}

TEST(filter, alpha) {
    AlphaFilter af;
    EXPECT_TRUE(af.filter("abcd"));
    EXPECT_FALSE(af.filter("ab43"));
    EXPECT_TRUE(af.filter(""));
}

TEST(filter, minlength) {
    MinLengthFilter mlf(4);
    EXPECT_TRUE(mlf.filter("abcd"));
    EXPECT_FALSE(mlf.filter("abc"));
}

TEST(transform, porter) {
    PorterTransform pt;
    EXPECT_EQ(pt.transform("sky"), "sky");
    EXPECT_EQ(pt.transform("gliding"), "glide");
}

TEST(transform, lower) {
    LowerTransform lt;
    EXPECT_EQ(lt.transform("sky"), "sky");
    EXPECT_EQ(lt.transform("sKy1"), "sky1");
}

TEST(tokenizer, simple) {
    BMITokenizer bmi_t;
    auto tokens = bmi_t.tokenize("hello dude");
    vector<string> tokens_exp = {"hello", "dude"};
    EXPECT_TRUE(tokens_exp == tokens);
}

TEST(tokenizer, sanitize) {
    BMITokenizer bmi_t;
    auto tokens = bmi_t.tokenize("heLlo, dude!!! gOing X 12s34");
    vector<string> tokens_exp = {"hello", "dude", "go"};
    EXPECT_TRUE(tokens_exp == tokens);
}

TEST(get_tf, basic) {
    BMITokenizer bmi_t;
    auto tokens = bmi_t.tokenize("heLlo, dude!!! gOing hello 12s34");
    auto ret = get_tf(tokens);

    EXPECT_EQ(ret.size(), 3);
    EXPECT_EQ(ret["hello"], 2);
    EXPECT_EQ(ret["dude"], 1);
    EXPECT_EQ(ret["go"], 1);
}
