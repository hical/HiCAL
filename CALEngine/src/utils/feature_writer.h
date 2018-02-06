#ifndef FEATURE_WRITER_H
#define FEATURE_WRITER_H

#include "features.h"
#include "../sofiaml/sf-sparse-vector.h"
#include "../dataset.h"
#include <fstream>
#include <memory>

class FeatureWriter{
    protected:
        static const char DELIM_CHAR = '\n';
        FILE *fp;
    public:
        FeatureWriter(const string &fname){ fp = fopen(fname.c_str(), "wb"); setvbuf(fp, nullptr, _IOFBF, 1 << 25); }
        virtual void write(const SfSparseVector &spv) = 0;
        void write_dataset(const Dataset &dataset);
        ~FeatureWriter(){fclose(fp);}
        virtual void finish() = 0;
};

class BinFeatureWriter:public FeatureWriter {
    uint32_t num_records = 0;
    uint32_t dict_end_offset;
    public:
        BinFeatureWriter(const string &file_name, const std::vector<std::pair<std::string, uint32_t>> &dictionary);
        void write(const SfSparseVector &spv) override;
        // Write final headers
        void finish() override;
};

class SVMlightFeatureWriter:public FeatureWriter {
    public:
        SVMlightFeatureWriter(const string &file_name, const string &df_file_name, const std::vector<std::pair<std::string, uint32_t>> &dictionary);
        void write(const SfSparseVector &spv) override;
        void finish() override {}
};
#endif // FEATURE_WRITER_H
