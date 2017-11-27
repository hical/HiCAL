#ifndef BMI_REDUCED_RANKING_H
#define BMI_REDUCED_RANKING_H

#include <mutex>
#include "bmi.h"

class BMI_reduced_ranking:public BMI {
    private:
    Dataset *subset;
    vector<int> indices;
    size_t subset_size;
    size_t refresh_period;
    size_t next_refresh;
    std::vector<int> perform_training_iteration();

    public:
    BMI_reduced_ranking(const Seed &seed,
        Dataset *documents,
        int num_threads,
        int judgments_per_iteration,
        int max_effort,
        int max_iterations,
        bool async_mode,
        size_t subset_size,
        size_t refresh_period);
};

#endif // BMI_REDUCED_RANKING_H
