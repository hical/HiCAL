#ifndef FEATURE_PARSER_H
#define FEATURE_PARSER_H

#include "features.h"
#include "../sofiaml/sf-sparse-vector.h"
#include <fstream>
#include <memory>
#include <unordered_map>

class FeatureParser{
    protected:
        static const char DELIM_CHAR = '\n';
        FILE *fp;
        std::unordered_map<std::string, TermInfo> dictionary;
    public:
        FeatureParser(const string &fname){ fp = fopen(fname.c_str(), "rb"); setvbuf(fp, NULL, _IOFBF, 1 << 25); }
        virtual std::unique_ptr<SfSparseVector> next() = 0;
        std::unordered_map<std::string, TermInfo> get_dictionary() { return dictionary; }

        ~FeatureParser(){fclose(fp);}
};

class BinFeatureParser:public FeatureParser {
    uint32_t num_records;
    public:
        BinFeatureParser(const string &file_name);
        BinFeatureParser(const string &file_name, const string &df_file_name);
        std::unique_ptr<SfSparseVector> next() override;
};

class SVMlightFeatureParser:public FeatureParser {
    char *buffer;
    size_t buffer_size;
    public:
        SVMlightFeatureParser(const string &file_name, const string &df_file_name);
        bool read_line();
        std::unique_ptr<SfSparseVector> next() override;
        ~SVMlightFeatureParser(){delete [] buffer;}
};
#endif // FEATURE_PARSER_H
