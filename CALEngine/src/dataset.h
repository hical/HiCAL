#ifndef DATASET_H
#define DATASET_H

#include <memory>
#include <unordered_map>
#include <map>
#include <queue>
#include <mutex>
#include "sofiaml/sf-sparse-vector.h"
#include "utils/features.h"
#include "utils/feature_parser.h"

typedef std::unordered_map<std::string, TermInfo> Dictionary;
typedef std::vector<std::pair<SfSparseVector, int>> Seed;
class Dataset {
    protected:
    std::unique_ptr<std::vector<std::unique_ptr<SfSparseVector>>> doc_features;
    Dictionary dictionary;
    const uint32_t dimensionality;
    const std::unordered_map<std::string, size_t> doc_ids_inv_map; // Inverted map of all document ids to their indices

    virtual void score_docs_priority_queue(const std::vector<float> &weights,
                                   int st, int end,
                                   std::priority_queue<std::pair<float, int>> &top_docs,
                                   std::mutex &top_docs_mutex,
                                   int num_top_docs,
                                   const std::map<int, int> &judgments);

    public:
    uint32_t NPOS;
    Dataset(std::unique_ptr<std::vector<std::unique_ptr<SfSparseVector>>>, Dictionary);
    Dataset():doc_features(nullptr), dimensionality(0), NPOS(0){};
    virtual float inner_product(size_t index, const std::vector<float> &weights) const;
    virtual std::vector<int> rescore(const vector<float> &weights,
                            int num_threads, int num_top_docs,
                            const std::map<int, int> &judgments);

    // Returns the index given the document id. return Dataset::NPOS if not found
    size_t get_index(const std::string &id) const {
        auto result = doc_ids_inv_map.find(id);
        if ( result == doc_ids_inv_map.end() ) return NPOS;
        return result->second;
    }

    const std::string &get_id(size_t idx){
        return doc_features->at(idx)->doc_id;
    }

    virtual const SfSparseVector& get_sf_sparse_vector(size_t index) const {
        return *(doc_features->at(index));
    }

    virtual size_t size() const {
        return doc_features->size();
    }

    size_t get_dimensionality() const {
        return dimensionality;
    }

    const Dictionary& get_dictionary() const {
        return dictionary;
    }

    virtual int translate_index(int id) const {return id;}

    static std::unique_ptr<Dataset> build(FeatureParser *feature_parser){
        auto sparse_feature_vectors = std::make_unique<vector<std::unique_ptr<SfSparseVector>>>();
        std::unique_ptr<SfSparseVector> spv;
        while((spv = feature_parser->next()) != nullptr)
            sparse_feature_vectors->push_back(std::move(spv));
        return std::make_unique<Dataset>(move(sparse_feature_vectors), feature_parser->get_dictionary());
    }
};

class ParagraphDataset:public Dataset {
    const Dataset &parent_dataset;
    vector<int> parent_documents;

    protected:
    void score_docs_priority_queue(const std::vector<float> &weights,
                                   int st, int end,
                                   std::priority_queue<std::pair<float, int>> &top_docs,
                                   std::mutex &top_docs_mutex,
                                   int num_top_docs,
                                   const std::map<int, int> &judgments);

    public:
    ParagraphDataset(const Dataset &_parent_dataset,
                    std::unique_ptr<std::vector<std::unique_ptr<SfSparseVector>>>,
                    Dictionary);
    virtual int translate_index(int id) const {return parent_documents[id];}
    std::vector<int> rescore(const vector<float> &weights,
                            int num_threads, int num_top_docs,
                            const std::map<int, int> &judgments);

    static std::unique_ptr<ParagraphDataset> build(FeatureParser *feature_parser, const Dataset &parent_dataset){
        auto sparse_feature_vectors = std::make_unique<vector<std::unique_ptr<SfSparseVector>>>();
        std::unique_ptr<SfSparseVector> spv;
        while((spv = feature_parser->next()) != nullptr)
            sparse_feature_vectors->push_back(std::move(spv));
        return std::make_unique<ParagraphDataset>(parent_dataset, move(sparse_feature_vectors), feature_parser->get_dictionary());
    }
};

class Dataset_subset:public Dataset {
    Dataset *d;
    std::vector<int> indices;
public:
    Dataset_subset(Dataset &_d, std::vector<int> _indices): d(&_d),indices(_indices) {
        NPOS = indices.size();
    }

    // Returns the inner product of `weights` with the sparse vector at `index`
    float inner_product(size_t index, const std::vector<float> &weights) const override {
        return d->inner_product(indices[index], weights);
    }

    const SfSparseVector& get_sf_sparse_vector(size_t index) const override {
        return d->get_sf_sparse_vector(indices[index]);
    }

    size_t size() const override {
        return indices.size();
    }

    int translate_index(int id) const override {return indices[id];}
};

#endif // DATASET_H
