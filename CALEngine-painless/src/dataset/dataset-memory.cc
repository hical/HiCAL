#include <fstream>
#include <sstream>
#include "dataset/dataset-memory.h"

using namespace std;

DatasetMemory::DatasetMemory(unique_ptr<Featurizer> featurizer,
                             const string &filename,
                             DatasetFormat dataset_format)
    : Dataset(move(featurizer)) {
  if (dataset_format == SVMLIGHT)
    load_from_svmlight(filename);
  else if (dataset_format == BIN) {
    load_from_bin(filename);
  }
}

size_t DatasetMemory::size() {
    return docs_.size();
}

void DatasetMemory::add_doc(const string &id, const string &text) {
    doc_index_[id] = docs_.size();
    auto fv = featurizer_->get_features(text);
    fv.doc_id = id;
    docs_.push_back(fv);
}

SfSparseVector DatasetMemory::get_features(const string &id) {
    return docs_[doc_index_[id]];
}

SfSparseVector DatasetMemory::get_features(int id) {
    return docs_[id];
}

void DatasetMemory::write(const string &filename, DatasetFormat dataset_format) {
  if (dataset_format == SVMLIGHT)
    write_to_svmlight(filename);
  else if (dataset_format == BIN) {
    write_to_bin(filename);
  }
}

void DatasetMemory::write_to_svmlight(const string &filename) {
    ofstream fout(filename);
    for(auto &spv: docs_){
        fout << spv.doc_id;
        for(auto &fpv: spv.features_){
            fout << " " << fpv.id_ << ":" << fpv.value_;
        }
        fout << "\n";
    }
}

void DatasetMemory::write_to_bin(const string &filename) {
    (void)filename;
}

void DatasetMemory::load_from_svmlight(const string &filename) {
    ifstream fin(filename);
    string line, doc;
    uint32_t f_id;
    float f_value;
    char delim;
    while(getline(fin, line)) {
        istringstream iss(line);
        iss >> doc;
        doc_index_[doc] = docs_.size();
        docs_.push_back(SfSparseVector(doc));
        while(iss >> f_id >> delim >> f_value) {
            docs_.back().PushPair({f_id, f_value});
        }
    }
}

void DatasetMemory::load_from_bin(const string &filename) {
    (void)filename;
}
