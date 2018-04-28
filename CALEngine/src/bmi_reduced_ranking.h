#ifndef BMI_REDUCED_RANKING_H
#define BMI_REDUCED_RANKING_H

#include <mutex>
#include "bmi.h"

class BMI_reduced_ranking:public BMI {
    protected:
    Dataset *subset;
    vector<int> indices;
    size_t subset_size;
    size_t refresh_period;

    protected:
    virtual bool is_it_refresh_time() {
        return (state.cur_iteration % refresh_period == 0);
    }

    public:
    BMI_reduced_ranking(Seed seed,
        Dataset *documents,
        int num_threads,
        int judgments_per_iteration,
        int max_effort,
        int max_iterations,
        bool async_mode,
        size_t subset_size,
        size_t refresh_period,
        int training_iterations);
    std::vector<int> perform_training_iteration();
};

#endif // BMI_REDUCED_RANKING_H
