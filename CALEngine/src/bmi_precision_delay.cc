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
        float _delta,
        float _noise_factor)
    :BMI(_seed, _documents, _num_threads, 10000, _max_effort, _max_iterations, _async_mode, false),
    noise_factor(_noise_factor), delta(_delta)
{
    perform_iteration();
    tot = 0;
    rel = 0;
    cur_p = 1.0;
}

void BMI_precision_delay::record_judgment_batch(std::vector<std::pair<std::string, int>> _judgments){
    for(const auto &judgment: _judgments){
        size_t id = documents->get_index(judgment.first);
        add_to_training_cache(id, judgment.second);
        remove_from_judgment_list(id);
        tot+=1;
        if(judgment.second > 0)
            rel += 1;
    }

    if(!async_mode){
        if(tot >= noise_factor / cur_p && (rel / tot < cur_p)){
            std::cerr<<"Time to retrain: P="<<cur_p<<" tot="<<tot<<std::endl;
            cur_p *= delta;
            tot = rel = 0;
            perform_iteration();
        }
    }else{
        auto t = std::thread(&BMI_precision_delay::perform_iteration_async, this);
        t.detach();
    }
}
