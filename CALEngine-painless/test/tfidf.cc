#include "featurizer/tfidf.h"
#include "gtest/gtest.h"

TEST(tfidf, basic) {
  TFIDFFeaturizer tfidf;
  tfidf.fit("hello dude, What's up?");
  tfidf.fit("hello mate, L.O.L dawg!", true);
  auto sf = tfidf.get_features("Ajdskjsd mate, dawg hello?");
  EXPECT_EQ(sf.features_.size(), 4);

  EXPECT_EQ(sf.features_[0].id_, 0);
  EXPECT_EQ(sf.features_[1].id_, 3);
  EXPECT_EQ(sf.features_[2].id_, 5);
  EXPECT_EQ(sf.features_[3].id_, 6);

  EXPECT_NEAR(sf.features_[0].value_, 1, 1e-6);
  EXPECT_NEAR(sf.features_[1].value_, 0, 1e-6);
  EXPECT_NEAR(sf.features_[2].value_, 0.0346574, 1e-6);
  EXPECT_NEAR(sf.features_[3].value_, 0.0346574, 1e-6);
}

TEST(tfidf, io) {
  TFIDFFeaturizer tfidf1;
  tfidf1.fit("hello dude, What's up?");
  tfidf1.fit("hello mate, L.O.L dawg!", true);
  tfidf1.write("tfidf.txt");
  TFIDFFeaturizer tfidf2("tfidf.txt");
  auto sf1 = tfidf1.get_features("Ajdskjsd mate, dawg hello?");
  auto sf2 = tfidf2.get_features("Ajdskjsd mate, dawg hello?");
  EXPECT_EQ(sf1.features_.size(), sf2.features_.size());
  EXPECT_EQ(sf1.doc_id, sf2.doc_id);
  for(size_t i = 0; i < sf1.features_.size(); i++){
      EXPECT_EQ(sf1.features_[i].id_, sf2.features_[i].id_);
      EXPECT_NEAR(sf1.features_[i].value_, sf2.features_[i].value_, 1e-6);
  }
}
