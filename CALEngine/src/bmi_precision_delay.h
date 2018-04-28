#ifndef BMI_PRECISION_DELAY_H
#define BMI_PRECISION_DELAY_H

#include "bmi.h"

class BMI_precision_delay:public BMI {
    protected:
    float threshold;
    int window;
    std::queue<int> q;
    int rel = 0;
    int tot = 0;
    void record_judgment_batch(std::vector<std::pair<std::string, int>> _judgments);
    std::vector<int> fetch_more();
    vector<float> weights;

    public:
    BMI_precision_delay(Seed seed,
        Dataset *documents,
        int num_threads,
        int max_effort,
        int max_iterations,
        bool async_mode,
        float threshold,
        int window,
        int training_iterations);
};

#endif // BMI_PRECISION_DELAY_H
