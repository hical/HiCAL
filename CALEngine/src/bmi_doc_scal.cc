#include <iostream>
#include <fstream>
#include <algorithm>
#include "bmi_doc_scal.h"
using namespace std;

BMI_doc_scal::BMI_doc_scal(Seed _seed,
        Dataset *_documents,
        int _num_threads,
        int _judgments_per_iteration,
        bool _async_mode,
        int _training_iterations,
        int _N)
    :BMI(_seed, _documents, _num_threads, _judgments_per_iteration,
       _async_mode,_training_iterations, false)
{
    N = _N;
    T = N;
    R = 0;
    judgments_per_iteration = B;
    perform_iteration();
}

void BMI_doc_scal::record_judgment_batch(vector<pair<string, int>> _judgments){
    for(const auto &judgment: _judgments){
        size_t id = documents->get_index(judgment.first);
        add_to_training_cache(id, judgment.second);
        if(judgment.second > 0) R++;
    }
    
    if(judgments.size() + training_cache.size() >= state.next_iteration_target){
        perform_iteration();
    }
    
}

void BMI_doc_scal::perform_iteration(){
    lock_guard<mutex> lock(state_mutex);
    auto results = perform_training_iteration();

    cerr<<"Fetched "<<results.size()<<" documents"<<endl;
    cerr<<"R = "<<R<<" B = "<<B<<" T = "<<T<<endl;
    if(R >= T) {
        T <<= 1;
        cerr<<"Doubling T to "<<T<<endl;
    }
    int n = ceil(B*N/(float)T);
    vector<int> selector(results.size());
    vector<int> samples;
    for(int i = 0; i < selector.size(); i++)
        selector[i] = (i < n?1:0);
    for(int i = 0; i < results.size(); i++){
        if(selector[i]) samples.push_back(results[i]);
    }

    cerr<<"Sampling "<<samples.size()<<" documents"<<endl;
    add_to_judgment_list(samples);
    if(!async_mode){
        state.next_iteration_target = min(state.next_iteration_target + n, (uint32_t)get_dataset()->size());
        
        judgments_per_iteration += (judgments_per_iteration + 9)/10;
        B = judgments_per_iteration;
    }
    for(int id : results)
        docs_in_strata[id] = -2;
    state.cur_iteration++;
}

vector<int> BMI_doc_scal::perform_training_iteration(){
    lock_guard<mutex> lock_training(training_mutex);

    sync_training_cache();

    // Training
    auto weights = train();

    // Scoring
    auto results = documents->rescore(weights, num_threads,
                              judgments_per_iteration + (async_mode ?
                                extra_judgment_docs : 0), docs_in_strata);

    return results;
}


    

