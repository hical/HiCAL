#include <iostream>
#include <mutex>
#include <thread>
#include <algorithm>
#include "bmi.h"
#include "scorer.h"

using namespace std;
BMI::BMI(const SfSparseVector &_seed,
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
    if(initialize)
        perform_iteration();
}

void BMI::finish_session(){
    add_to_judgment_list({-1});
    state.finished = true;
}

void BMI::perform_iteration(){
    if(max_iterations != -1 && state.cur_iteration >= max_iterations){
        finish_session();
    }

    if(!state.finished){
        lock_guard<mutex> lock(state_mutex);
        cerr<<"Beginning Iteration "<<state.cur_iteration<<endl;
        auto results = perform_training_iteration();
        cerr<<"Fetched "<<results.size()<<" documents"<<endl;
        add_to_judgment_list(results);
        state.next_iteration_target += judgments_per_iteration;
        if(is_bmi)
            judgments_per_iteration += (judgments_per_iteration + 9)/10;
        state.cur_iteration++;
    }
}

void BMI::perform_iteration_async(){
    if(async_training_mutex.try_lock()){
        while(!training_cache.empty()){
            if(max_iterations != -1 && state.cur_iteration >= max_iterations){
                finish_session();
                return;
            }

            if(!state.finished){
                lock_guard<mutex> lock(state_mutex);
                cerr<<"Beginning Iteration "<<state.cur_iteration<<endl;
                auto results = perform_training_iteration();
                cerr<<"Fetched "<<results.size()<<" documents"<<endl;
                add_to_judgment_list(results);
                state.cur_iteration++;
            }
        }
        async_training_mutex.unlock();
    }
}

void BMI::train(SfWeightVector &w){
    vector<const SfSparseVector*> positives, negatives;
    positives.push_back(&seed);
    // Sampling random non_rel documents
    uniform_int_distribution<size_t> distribution(0, documents->size()-1);
    for(int i = 1;i<=100;i++){
        size_t idx = distribution(rand_generator);
        negatives.push_back(&documents->get_sf_sparse_vector(idx));
    }

    for(pair<int, int> judgment: judgments){
        if(judgment.second > 0)
            positives.push_back(&documents->get_sf_sparse_vector(judgment.first));
        else
            negatives.push_back(&documents->get_sf_sparse_vector(judgment.first));
    }

    std::cerr<<"Training on "<<positives.size()<<" +ve docs and "<<negatives.size()<<" -ve docs"<<std::endl;
    
    sofia_ml::StochasticRocLoop(positives,
            negatives,
            sofia_ml::LOGREG_PEGASOS,
            sofia_ml::PEGASOS_ETA,
            0.0001,
            10000000.0,
            200000,
            &w);
}

vector<string> BMI::get_doc_to_judge(uint32_t count=1){
    while(true){
        {
            lock_guard<mutex> lock(judgment_list_mutex);
            lock_guard<mutex> lock2(training_cache_mutex);
            if(!judgment_list.empty()){
                vector<string> ret;
                for(int id: judgment_list){
                    if(id == -1 || ret.size() >= count)
                        break;
                    ret.push_back(documents->get_sf_sparse_vector(id).doc_id);
                }
                return ret;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void BMI::add_to_judgment_list(const vector<int> &ids){
    lock_guard<mutex> lock(judgment_list_mutex);
    judgment_list = ids;
}

void BMI::wait_for_judgments(){
    while(1){
        {
            // NOTE: avoid deadlock by making sure whichever thread locks these two mutexes,
            // they always do in the same order
            lock_guard<mutex> lock(judgment_list_mutex);
            if(judgment_list.empty())
                return;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void BMI::add_to_training_cache(int id, int judgment){
    lock_guard<mutex> lock(training_cache_mutex);
    training_cache[id] = judgment;
}

void BMI::remove_from_judgment_list(int id){
    lock_guard<mutex> lock(judgment_list_mutex);
    judgment_list.erase(std::remove(judgment_list.begin(), judgment_list.end(), id));
}

void BMI::record_judgment_batch(vector<pair<string, int>> _judgments){
    for(const auto &judgment: _judgments){
        size_t id = documents->get_index(judgment.first);
        add_to_training_cache(id, judgment.second);
        remove_from_judgment_list(id);
    }

    if(!async_mode){
        if(finished_judgments.size() + training_cache.size() >= state.next_iteration_target)
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
            finished_judgments.insert(training.first);
        }
        training_cache.clear();
    }

    // Training
    auto start = std::chrono::steady_clock::now();

    SfWeightVector w(documents->get_dimensionality());
    train(w);

    auto weights = w.AsFloatVector();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Training finished in "<<duration.count()<<"ms"<<endl;

    vector<int> results;

    // Scoring
    start = std::chrono::steady_clock::now();
    Scorer::rescore_documents(*documents, weights, num_threads, judgments_per_iteration+(async_mode?extra_judgment_docs:0), finished_judgments, results);
    duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Rescored "<<documents->size()<<" documents in "<<duration.count()<<"ms"<<endl;

    state.weights = weights;

    return results;
}

std::vector<std::pair<string, float>> BMI::get_ranklist(){
    vector<std::pair<string, float>> ret_results;
    auto results = Scorer::rescore_all_documents(*get_ranking_dataset(), state.weights, num_threads);
    for(auto result: results){
        ret_results.push_back({get_ranking_dataset()->get_sf_sparse_vector(result.first).doc_id, result.second});
    }
    return ret_results;
}