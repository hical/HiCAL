#ifndef BMI_REDUCED_RANKING_DYNAMIC_H
#define BMI_REDUCED_RANKING_DYNAMIC_H

#include <mutex>
#include "bmi_reduced_ranking.h"

class BMI_reduced_ranking_dynamic:public BMI_reduced_ranking {
    int next_refresh = 1;
    int cur_subset_size = ;
    protected:
    bool is_it_refresh_time() {
        return (state.cur_iteration % refresh_period == 0);
    }

    public:
    BMI_reduced_ranking_dynamic(const Seed &seed,
                                Dataset *documents,
                                int num_threads,
                                int judgments_per_iteration,
                                int max_effort,
                                int max_iterations,
                                bool async_mode,
                                size_t subset_size)
        :BMI_reduced_ranking(seed, documents, num_threads, judgments_per_iteration, max_effort, max_iterations, async_mode, subset_size, -1)
    {
        
    };
};

#endif // BMI_REDUCED_RANKING_DYNAMIC_H
