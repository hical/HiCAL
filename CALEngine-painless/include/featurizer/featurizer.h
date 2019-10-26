#ifndef FEATURIZER_H
#define FEATURIZER_H

#include <string>
#include "utils/sf-sparse-vector.h"
#include "utils/logging.h"

class Featurizer {
public:
  virtual void fit(const std::string &text) = 0;

  virtual void write(const std::string &filename) = 0;

  virtual SfSparseVector get_features(const std::string &text) = 0;

  virtual void finalize() {};

  virtual ~Featurizer() {}
};

#endif // FEATURIZER_H
