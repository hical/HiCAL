#include "feature_parser.h"
#include <cstring>

using namespace std;

BinFeatureParser::BinFeatureParser(const string &file_name): FeatureParser(file_name){
    uint32_t dict_end_offset;
    fread(&dict_end_offset, sizeof(uint32_t), 1, fp);
    char buffer[MAX_TERM_LEN];
    uint32_t df;
    int idx = 0;
    while(ftello(fp) != dict_end_offset){
        char ch; int buf_idx = 0;
        do{
            ch = fgetc(fp);
            buffer[buf_idx++] = ch;
        } while(ch);

        fread(&df, sizeof(uint32_t), 1, fp);
        dictionary[buffer] = { ++idx, (int)df};
    }
    fread(&num_records, sizeof(num_records), 1, fp);
}

BinFeatureParser::BinFeatureParser(const string &file_name, const string &df_file_name): FeatureParser(file_name){
    char buffer[1<<10];
    if(df_file_name.size() > 0){
        FILE *df_fp = fopen(df_file_name.c_str(), "rb");
        int df, idx = 0;
        while(fscanf(df_fp, "%d %s\n", &df, buffer) != EOF){
            dictionary[buffer] = { ++idx, df};
        }
        fclose(df_fp);
    }

    fread(&num_records, sizeof(num_records), 1, fp);
}

SVMlightFeatureParser::SVMlightFeatureParser(const string &file_name, const string &df_file_name): FeatureParser(file_name){
    buffer_size = 1<<10;
    buffer = (char*)malloc(buffer_size);

    if(df_file_name.size() > 0){
        FILE *df_fp = fopen(df_file_name.c_str(), "rb");
        int df, idx = 0;
        while(fscanf(df_fp, "%d %s\n", &df, buffer) != EOF){
            dictionary[buffer] = { ++idx, df};
        }
        fclose(df_fp);
    }
}

// Bad things will happen if the file is corrupted
// Todo: move things to heap
std::unique_ptr<SfSparseVector> BinFeatureParser::next(){
    string doc_id;
    char c = fgetc(fp);
    if(c == EOF)
        return NULL;
    while(c != DELIM_CHAR){
        doc_id.push_back(c);
        c = fgetc(fp);
    }

    uint32_t num_pairs = 0;
    fread(&num_pairs, sizeof(num_pairs), 1, fp);

    vector<FeatureValuePair> features(num_pairs);
    for(auto &fvp: features){
        uint32_t x;
        fread(&x, sizeof(uint32_t), 1, fp);
        fvp.id_ = x;
        fread(&fvp.value_, sizeof(float), 1, fp);
    }
    return std::make_unique<SfSparseVector>(doc_id, features);
}

bool SVMlightFeatureParser::read_line(){
    if(feof(fp))
        return false;
    char *cur_pos = buffer;
    size_t read_size = buffer_size;
    while(1){
        buffer[buffer_size-2] = 0;
        if(!fgets(cur_pos, read_size, fp)){
            return (cur_pos != buffer);
        }
        if(buffer[buffer_size-2] == 0 || buffer[buffer_size-2] == '\n'){
            break;
        }else{
            buffer = (char*)realloc(buffer, buffer_size << 1);
            cur_pos = buffer + buffer_size - 1;
            read_size = buffer_size + 1;
            buffer_size <<= 1;
        }
    }
    return true;
}

// Move to c file buffers
// Bad things will happen if file is not proper!
std::unique_ptr<SfSparseVector> SVMlightFeatureParser::next(){
    if(!read_line())
        return nullptr;

    string doc_id;
    vector<FeatureValuePair> features;
    char *moving_chr = buffer;
    while(*moving_chr != ' ' && *moving_chr != 0){
        doc_id.push_back(*moving_chr);
        moving_chr++;
    }

    while(*moving_chr && *moving_chr != '\n'){
        moving_chr++;
        uint32_t feature_id = atoi(moving_chr);
        moving_chr = strchr(moving_chr, ':') + 1;
        float feature_weight = atof(moving_chr);
        moving_chr = strchr(moving_chr, ' ');

        features.push_back({feature_id, feature_weight});
        if(moving_chr == nullptr)
            break;
    }
    return std::make_unique<SfSparseVector>(doc_id, features);
}
