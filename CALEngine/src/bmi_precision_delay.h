#ifndef BMI_PRECISION_DELAY_H
#define BMI_PRECISION_DELAY_H

#include "bmi.h"

class BMI_precision_delay:public BMI {
    protected:
    float delta, noise_factor;
    float cur_p;
    float tot, rel;
    void record_judgment_batch(std::vector<std::pair<std::string, int>> _judgments);

    public:
    BMI_precision_delay(Seed seed,
        Dataset *documents,
        int num_threads,
        int max_effort,
        int max_iterations,
        bool async_mode,
        float delta,
        float noise_factor);
};

#endif // BMI_PRECISION_DELAY_H
