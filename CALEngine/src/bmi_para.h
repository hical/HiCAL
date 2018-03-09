#ifndef BMI_PARA_H
#define BMI_PARA_H

#include <random>
#include <mutex>
#include <set>
#include "bmi.h"

class BMI_para:public BMI {
    private:
    Dataset *paragraphs;
    std::map<int, int> finished_judgments_para;

    public:
    BMI_para(Seed seed,
        Dataset *documents,
        Dataset *paragraphs,
        int num_threads,
        int judgments_per_iteration,
        int max_effort,
        int max_iterations,
        bool async_mode);

    void record_judgment(std::string doc_id, int judgment);
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
