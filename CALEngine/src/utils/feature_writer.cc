#include "feature_writer.h"

BinFeatureWriter::BinFeatureWriter(const string &file_name): FeatureWriter(file_name){
    fseeko(fp, sizeof(uint32_t), SEEK_CUR);
}

SVMlightFeatureWriter::SVMlightFeatureWriter(const string &file_name): FeatureWriter(file_name){
}

void BinFeatureWriter::write(const SfSparseVector &spv){
    fwrite(spv.doc_id.c_str(), 1, spv.doc_id.length(), fp);
    fputc(DELIM_CHAR, fp);

    uint32_t num_pairs = 0;
    off_t record_len_offset = ftello(fp);
    fseeko(fp, sizeof(uint32_t), SEEK_CUR);

    for(auto &fpv: spv.features_){
        if(fpv.id_ != 0){
            num_pairs++;
            fwrite(&fpv.id_, sizeof(uint32_t), 1, fp);
            fwrite(&fpv.value_, sizeof(float), 1, fp);
        }
    }

    off_t backup_offset = ftello(fp);
    fseeko(fp, record_len_offset, SEEK_SET);
    fwrite(&num_pairs, sizeof(uint32_t), 1, fp);
    fseeko(fp, backup_offset, SEEK_SET);
    num_records++;
}

void BinFeatureWriter::finish(){
    fseeko(fp, 0, SEEK_SET);
    fwrite(&num_records, sizeof(uint32_t), 1, fp);
    fflush(fp);
}

void SVMlightFeatureWriter::write(const SfSparseVector &spv){
    fprintf(fp, "%s", spv.doc_id.c_str());
    for(auto &fpv: spv.features_){
        if(fpv.id_ != 0)
            fprintf(fp, " %d:%.8f", fpv.id_, fpv.value_);
    }
    fprintf(fp, "\n");
}

void FeatureWriter::write_dataset(const Dataset &dataset) {
    for(size_t i = 0; i < dataset.size(); i++){
        write(dataset.get_sf_sparse_vector(i));
    }
}
