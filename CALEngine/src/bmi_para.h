#ifndef BMI_PARA_H
#define BMI_PARA_H

#include <random>
#include <mutex>
#include <set>
#include "sofiaml/sofia-ml-methods.h"
#include "bmi.h"

class BMI_para:public BMI {
    private:
    Dataset *paragraphs;
    std::set<int> finished_judgments_para;
    std::vector<int> perform_training_iteration();
    virtual void remove_from_judgment_list(int id);

    public:
    BMI_para(const SfSparseVector &seed,
        Dataset *documents,
        Dataset *paragraphs,
        int num_threads,
        int judgments_per_iteration,
        int max_effort,
        int max_iterations,
        bool async_mode);

    std::vector<std::string> get_doc_to_judge(uint32_t count);
    void record_judgment(std::string doc_id, int judgment);
    Dataset *get_ranking_dataset() {return paragraphs;};
    vector<std::pair<string, float>> get_ranklist();
};

#endif // BMI_PARA_H
