#include "dataset/dataset-memory.h"

#include <fstream>
#include <sstream>

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

DatasetMemory::DatasetMemory(unique_ptr<Featurizer> featurizer)
    : Dataset(move(featurizer)) {}

void DatasetMemory::add(const string &id, const string &text) {
    index_[id] = items_.size();
    items_.push_back({id, featurizer_->transform(text)});
}

const Dataset::DatasetItem &DatasetMemory::get(const string &id) const {
    return items_[index_.at(id)];
}

void DatasetMemory::write(const string &filename,
                          DatasetFormat dataset_format) const {
    if (dataset_format == SVMLIGHT)
        write_to_svmlight(filename);
    else if (dataset_format == BIN) {
        write_to_bin(filename);
    }
}

void DatasetMemory::write_to_svmlight(const string &filename) const {
    ofstream fout(filename);
    for (auto &item : items_) {
        fout << item.id;
        for (auto &fpv : item.featureVector.features_) {
            fout << " " << fpv.id_ << ":" << fpv.value_;
        }
        fout << "\n";
    }
}

void DatasetMemory::write_to_bin(const string &filename) const {
    (void)filename;
}

void DatasetMemory::load_from_svmlight(const string &filename) {
    ifstream fin(filename);
    string line, doc;
    uint32_t f_id;
    float f_value;
    char delim;
    while (getline(fin, line)) {
        istringstream iss(line);
        iss >> doc;
        index_[doc] = items_.size();
        items_.push_back({doc, SfSparseVector()});
        while (iss >> f_id >> delim >> f_value) {
            items_.back().featureVector.PushPair({f_id, f_value});
        }
    }
}

void DatasetMemory::load_from_bin(const string &filename) { (void)filename; }
