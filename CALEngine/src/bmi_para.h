#ifndef BMI_PARA_H
#define BMI_PARA_H

#include <random>
#include <mutex>
#include <set>
#include "scorer.h"
#include "sofiaml/sofia-ml-methods.h"
#include "sofiaml/sf-data-set.h"
#include "bmi.h"

class BMI_para:public BMI {
    private:
    Scorer *scorer_para;
    std::set<int> finished_judgments_para;
    std::vector<int> perform_training_iteration();

    public:
    BMI_para(const SfSparseVector &seed,
        Scorer *scorer,
        Scorer *scorer_para,
        int num_threads,
        int judgments_per_iteration,
        int max_effort,
        int max_iterations,
        bool async_mode);

    std::vector<std::string> get_doc_to_judge(uint32_t count);
    void record_judgment(std::string doc_id, int judgment);
};

#endif // BMI_PARA_H
