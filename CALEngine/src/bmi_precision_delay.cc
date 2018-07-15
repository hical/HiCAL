#include <iostream>
#include <mutex>
#include <thread>
#include "bmi_precision_delay.h"
#include "utils/utils.h"
using namespace std;
BMI_precision_delay::BMI_precision_delay(Seed _seed,
        Dataset *_documents,
        int _num_threads,
        bool _async_mode,
        float _threshold,
        int _window,
        int _training_iterations)
    :BMI(_seed, _documents, _num_threads, 1, _async_mode, _training_iterations, false),
    threshold(_threshold), window(_window)
{
    perform_iteration();
}

vector<int> BMI_precision_delay::perform_training_iteration(){
    lock_guard<mutex> lock_training(training_mutex);

    sync_training_cache();

    // Training
    if(!skip_training){
        TIMER_BEGIN(training);
        weights = train();
        TIMER_END(training);
    }


    // Scoring
    TIMER_BEGIN(rescoring);
    auto results = documents->rescore(weights, num_threads,
                              judgments_per_iteration + (async_mode ? extra_judgment_docs : 0), judgments);
    TIMER_END(rescoring);

    return results;
}

void BMI_precision_delay::record_judgment_batch(std::vector<std::pair<std::string, int>> _judgments){
    state.next_iteration_target += 1;
    int last_rel;
    for(const auto &judgment: _judgments){
        size_t id = documents->get_index(judgment.first);
        if(judgment_queue.size() > 0 && id == judgment_queue.back()){
            judgment_queue.pop_back();
        }
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
            if(judgments_per_iteration > 1)
                judgments_per_iteration /= 2;
            perform_iteration();
        }else if(judgment_queue.size() == 0 || get_doc_to_judge(1).size() == 0){
            judgments_per_iteration *= 2;
            skip_training = true;
            add_to_judgment_list(perform_training_iteration());
            skip_training = false;
        }
    }else{
        auto t = std::thread(&BMI_precision_delay::perform_iteration_async, this);
        t.detach();
    }
}
