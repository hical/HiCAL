#ifndef BMI_PARA_H
#define BMI_PARA_H

#include <random>
#include <mutex>
#include "bmi.h"

class BMI_para:public BMI {
    protected:
    ParagraphDataset *paragraphs;

    public:
    BMI_para(Seed seed,
        Dataset *documents,
        ParagraphDataset *paragraphs,
        int num_threads,
        int judgments_per_iteration,
        bool async_mode,
        int training_iterations);

    virtual void record_judgment(std::string doc_id, int judgment);
    Dataset *get_ranking_dataset() {return paragraphs;};
    vector<std::pair<string, float>> get_ranklist();
    std::vector<int> perform_training_iteration();

    bool is_judged(int id) {
        std::string para = (paragraphs->get_sf_sparse_vector(id)).doc_id;
        id = documents->get_index(para.substr(0, para.find(".")));
        return judgments.find(id) != judgments.end() || training_cache.find(id) != training_cache.end();
    }
};

#endif // BMI_PARA_H
