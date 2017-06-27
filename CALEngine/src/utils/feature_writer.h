#ifndef FEATURE_WRITER_H
#define FEATURE_WRITER_H

#include "../sofiaml/sf-sparse-vector.h"
#include <fstream>

namespace CAL{
    namespace utils{
        class FeatureWriter{
            protected:
                static const char DELIM_CHAR = '\n';
                FILE *fp;
            public:
                FeatureWriter(std::string fname){ fp = fopen(fname.c_str(), "wb"); setvbuf(fp, NULL, _IOFBF, 1<<25); }
                virtual void write(const SfSparseVector*) = 0;
                ~FeatureWriter(){fclose(fp);}
        };

        class BinFeatureWriter:public FeatureWriter {
            uint32_t num_records = 0;
            public:
                BinFeatureWriter(std::string file_name);
                void write(const SfSparseVector*);
                // Write final headers
                void finish();
        };

        class SVMlightFeatureWriter:public FeatureWriter {
            public:
                SVMlightFeatureWriter(std::string file_name);
                void write(const SfSparseVector*);
        };
    }
}
#endif // FEATURE_WRITER_H
