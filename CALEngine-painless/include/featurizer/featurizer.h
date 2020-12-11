#ifndef FEATURIZER_H
#define FEATURIZER_H

#include <utils/logging.h>
#include <utils/sf-sparse-vector.h>

#include <string>

class Featurizer {
   public:
    virtual void fit(const std::string &text) = 0;

    virtual void write(const std::string &filename) const = 0;

    virtual SfSparseVector transform(const std::string &text) const = 0;

    virtual void finalize(){};

    virtual ~Featurizer() {}
};

#endif  // FEATURIZER_H
