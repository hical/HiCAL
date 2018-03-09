#include <iostream>
#include <mutex>
#include <thread>
#include <algorithm>
#include <climits>
#include "bmi.h"
#include "classifier.h"
#include "utils/utils.h"

using namespace std;
BMI::BMI(Seed _seed,
         Dataset *_documents,
         int _num_threads,
         int _judgments_per_iteration,
         int _max_effort,
         int _max_iterations,
         bool _async_mode,
         bool initialize)
    :documents(_documents),
    num_threads(_num_threads),
    judgments_per_iteration(_judgments_per_iteration),
    max_effort(_max_effort),
    max_iterations(_max_iterations),
    async_mode(_async_mode),
    seed(_seed)
{
    is_bmi = (judgments_per_iteration == -1);
    if(is_bmi || _async_mode)
        judgments_per_iteration = 1;
    if(max_effort < 0)
        max_effort = INT_MAX;
    if(initialize)
        perform_iteration();
}

void BMI::finish_session(){
    add_to_judgment_list({-1});
    state.finished = true;
}

bool BMI::try_finish_session() {
    lock_guard<mutex> lock(training_cache_mutex);
    if((max_iterations != -1 && state.cur_iteration >= max_iterations) \
        || (judgments.size() + training_cache.size()) >= min((int)get_dataset()->size(), max_effort)){
        finish_session();
        return true;
    }
    return state.finished;
}

void BMI::perform_iteration(){
    if(!try_finish_session()){
        lock_guard<mutex> lock(state_mutex);
        cerr<<"Beginning Iteration "<<state.cur_iteration<<endl;
        auto results = perform_training_iteration();
        cerr<<"Fetched "<<results.size()<<" documents"<<endl;
        add_to_judgment_list(results);
        if(!async_mode){
            state.next_iteration_target = min(state.next_iteration_target + judgments_per_iteration, (uint32_t)get_dataset()->size());
            if(is_bmi)
                judgments_per_iteration += (judgments_per_iteration + 9)/10;
        }
        state.cur_iteration++;
    }
}

void BMI::perform_iteration_async(){
    if(async_training_mutex.try_lock()){
        while(!training_cache.empty()){
            if(try_finish_session())
                return;
            perform_iteration();
        }
        async_training_mutex.unlock();
    }
}

vector<float> BMI::train(){
    vector<const SfSparseVector*> positives, negatives;
    for(auto &judgment: seed){
        if(judgment.second > 0)
            positives.push_back(&judgment.first);
        else
            negatives.push_back(&judgment.first);
    }

    // Sampling random non_rel documents
    uniform_int_distribution<size_t> distribution(0, documents->size()-1);
    for(int i = 1;i<=100;i++){
        size_t idx = distribution(rand_generator);
        negatives.push_back(&documents->get_sf_sparse_vector(idx));
    }

    for(const pair<int, int> &judgment: judgments){
        if(judgment.second > 0)
            positives.push_back(&documents->get_sf_sparse_vector(judgment.first));
        else
            negatives.push_back(&documents->get_sf_sparse_vector(judgment.first));
    }

    std::cerr<<"Training on "<<positives.size()<<" +ve docs and "<<negatives.size()<<" -ve docs"<<std::endl;
    
    LRPegasosClassifier().train(positives, negatives, &w);
    return w;
}

vector<string> BMI::get_doc_to_judge(uint32_t count=1){
    while(true){
        {
            lock_guard<mutex> lock(judgment_list_mutex);
            if(!judgment_queue_by_rank.empty()){
                vector<string> ret;
                for(pair<int, int> rank_id: judgment_queue_by_rank){
                    if(rank_id.second == -1 || ret.size() >= count)
                        break;
                    ret.push_back(get_ranking_dataset()->get_sf_sparse_vector(rank_id.second).doc_id);
                }
                return ret;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void BMI::add_to_judgment_list(const vector<int> &ids){
    lock_guard<mutex> lock(judgment_list_mutex);
    judgment_queue_by_id.clear();
    judgment_queue_by_rank.clear();
    for(int i = 0; i < ids.size(); i++){
        judgment_queue_by_id[ids[i]] = i;
        judgment_queue_by_rank[i] = ids[i];
    }
}

void BMI::add_to_training_cache(int id, int judgment){
    lock_guard<mutex> lock(training_cache_mutex);
    training_cache[id] = judgment;
}

void BMI::remove_from_judgment_list(int id){
    lock_guard<mutex> lock(judgment_list_mutex);
    auto it = judgment_queue_by_id.find(id);
    if(it == judgment_queue_by_id.end())
        return;
    judgment_queue_by_rank.erase(it->second);
    judgment_queue_by_id.erase(it);
}

void BMI::record_judgment_batch(vector<pair<string, int>> _judgments){
    for(const auto &judgment: _judgments){
        size_t id = documents->get_index(judgment.first);
        add_to_training_cache(id, judgment.second);
        remove_from_judgment_list(id);
    }

    if(!async_mode){
        if(judgments.size() + training_cache.size() >= state.next_iteration_target)
            perform_iteration();
    }else{
        auto t = thread(&BMI::perform_iteration_async, this);
        t.detach();
    }
}

void BMI::record_judgment(string doc_id, int judgment){
    record_judgment_batch({{doc_id, judgment}});
}

vector<int> BMI::perform_training_iteration(){
    lock_guard<mutex> lock_training(training_mutex);

    {
        lock_guard<mutex> lock(training_cache_mutex);
        for(pair<int, int> training: training_cache){
            judgments[training.first] = training.second;
        }
        training_cache.clear();
    }

    // Training
    TIMER_BEGIN(training);
    auto weights = train();
    TIMER_END(training);


    // Scoring
    TIMER_BEGIN(rescoring);
    auto results = documents->rescore(weights, num_threads,
                              judgments_per_iteration + (async_mode ? extra_judgment_docs : 0), judgments);
    TIMER_END(rescoring);

    return results;
}

std::vector<std::pair<string, float>> BMI::get_ranklist(){
    vector<std::pair<string, float>> ret_results;
    auto results = get_ranking_dataset()->rescore(train(), num_threads,
                              get_ranking_dataset()->size(), map<int, int>());

    // to fix
    for(auto result: results){
        ret_results.push_back({get_ranking_dataset()->get_sf_sparse_vector(result).doc_id, 0});
    }
    return ret_results;
}
