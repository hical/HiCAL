#include <iostream>
#include <mutex>
#include <thread>
#include "bmi_precision_delay.h"
BMI_precision_delay::BMI_precision_delay(Seed _seed,
        Dataset *_documents,
        int _num_threads,
        int _max_effort,
        int _max_iterations,
        bool _async_mode,
        float _threshold,
        int _window)
    :BMI(_seed, _documents, _num_threads, 10000, _max_effort, _max_iterations, _async_mode, false),
    threshold(_threshold), window(_window)
{
    perform_iteration();
}

void BMI_precision_delay::record_judgment_batch(std::vector<std::pair<std::string, int>> _judgments){
    int last_rel;
    for(const auto &judgment: _judgments){
        size_t id = documents->get_index(judgment.first);
        add_to_training_cache(id, judgment.second);
        q.push(judgment.second);
        tot++;
        if(q.size() > window){
            int x = q.front();
            q.pop();
            if(x > 0)
                rel -= 1;
        }
        if(judgment.second > 0)
            rel += 1;
        last_rel = judgment.second;
    }

    if(!async_mode){
        if(last_rel <= 0 && rel / (float)q.size() < threshold){
            std::cerr<<"Refreshing P"<<std::endl;
            perform_iteration();
        }
    }else{
        auto t = std::thread(&BMI_precision_delay::perform_iteration_async, this);
        t.detach();
    }
}
