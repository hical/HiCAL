#ifndef BMI_ONLINE_LEARNING_H
#define BMI_ONLINE_LEARNING_H

#include <mutex>
#include "bmi.h"

class BMI_online_learning:public BMI {
    protected:
    size_t refresh_period;
    vector<float> weight;
    float delta;

    protected:
    virtual bool is_it_refresh_time() {
        return (state.cur_iteration % refresh_period == 0) || weight.size() == 0;
    }

    public:
    BMI_online_learning(Seed seed,
        Dataset *documents,
        int num_threads,
        int judgments_per_iteration,
        bool async_mode,
        size_t refresh_period,
        float delta,
        int training_iterations);
    std::vector<int> perform_training_iteration();
};

#endif // BMI_ONLINE_LEARNING_H
