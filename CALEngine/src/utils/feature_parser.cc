#include "feature_parser.h"

#include <cstring>

CAL::utils::BinFeatureParser::BinFeatureParser(std::string file_name):FeatureParser(file_name){
    fread(&num_records, sizeof(num_records), 1, fp);
}

CAL::utils::SVMlightFeatureParser::SVMlightFeatureParser(std::string file_name):FeatureParser(file_name){
    buffer_size = 1<<10;
    buffer = (char*)malloc(buffer_size);
}

// Bad things will happen if the file is corrupted
// Todo: move things to heap
SfSparseVector* CAL::utils::BinFeatureParser::next(){
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
    return new SfSparseVector(doc_id, features);
}

bool CAL::utils::SVMlightFeatureParser::read_line(){
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
SfSparseVector* CAL::utils::SVMlightFeatureParser::next(){
    if(!read_line())
        return NULL;

    string doc_id = "";
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
        if(moving_chr == NULL)
            break;
    }
    return new SfSparseVector(doc_id, features);
}

vector<SfSparseVector*> CAL::utils::FeatureParser::get_all(){
    vector<SfSparseVector*> sparse_feature_vectors;
    SfSparseVector *spv;
    while((spv = next()) != NULL)
        sparse_feature_vectors.push_back(spv);
    return sparse_feature_vectors;
}
