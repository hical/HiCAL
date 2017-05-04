#include<iostream>
#include<chrono>
#include<algorithm>
#include<fstream>
#include<cstdlib>
#include<queue>
#include<mutex>
#include<chrono>
#include<thread>
#include<unordered_set>
#include<cassert>
#include "scorer.h"
#include "sofiaml/sofia-ml-methods.h"
#include "sofiaml/sf-data-set.h"

using namespace std;

queue<int> judgment_queue;
unordered_set<int> pending_judgments;

mutex judgment_queue_mutex;
mutex pending_judgments_mutex;;

string get_feature_string(SfSparseVector &sfv, int judgment){
    string ret = to_string(judgment);
    for(auto feature: sfv.features_){
        ret += " " + to_string(feature.id_) + ":" + to_string(feature.value_);
    }
    return ret;
}

void randomize_non_rel_docs(SfDataSet &training_data){
    for(int i = 1;i<=100;i++){
        int idx = rand() % doc_features.size();
        if(training_data.NumExamples() < i+1){
            training_data.AddLabeledVector(doc_features[idx], -1);
        }else{
            training_data.ModifyLabeledVector(i, doc_features[idx], -1);
        }
    }
}

SfDataSet get_initial_training_data(string fname){
    // Todo generalize seed docs
    SfDataSet training_data = SfDataSet(true);
    auto sparse_feature_vectors = parse_doc_features(fname);
    cout<<sparse_feature_vectors.size()<<endl;
    assert(sparse_feature_vectors.size() == 1);
    training_data.AddLabeledVector(sparse_feature_vectors[0], 1);
    return training_data;
}

void run_classifier(const SfDataSet& training_data, SfWeightVector &w, int dimensionality){
    sofia_ml::StochasticRocLoop(training_data,
			      sofia_ml::LOGREG_PEGASOS,
			      sofia_ml::PEGASOS_ETA,
			      0.0001,
			      10000000.0,
			      200000,
			      &w);
}

string get_doc_to_judge(){
    while(true){
        {
            lock_guard<mutex> lock(judgment_queue_mutex);
            if(judgment_queue.size() > 0){
                int id = judgment_queue.front();
                // poison value
                if(id == -1){
                    return "";
                }

                lock_guard<mutex> lock2(pending_judgments_mutex);
                pending_judgments.insert(id);
                judgment_queue.pop();

                return doc_features[id].doc_id;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void add_to_judgment_queue(const vector<int> &ids){
    lock_guard<mutex> lock(judgment_queue_mutex);
    for(int id: ids)
        judgment_queue.push(id);
}

void wait_for_judgments(){
    while(1){
        {
            // NOTE: avoid deadlock by making sure whichever thread locks these two mutexes,
            // they always do in the same order
            lock_guard<mutex> lock(judgment_queue_mutex);
            lock_guard<mutex> lock2(pending_judgments_mutex);
            if(pending_judgments.size() == 0 && judgment_queue.size() == 0)
                return;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

SfDataSet training_data(true);
void record_judgment(string doc_id, int judgment){
    ofstream fout("judgments", ios_base::app | ios_base::out);
    int id = doc_ids_inv_map[doc_id];
    training_data.AddLabeledVector(doc_features[id], judgment);
    lock_guard<mutex> lock(pending_judgments_mutex);
    pending_judgments.erase(id);
}

void run_bmi(string doc_features_path,
        string seed_path,
        int num_threads,
        int judgements_per_iteration,
        int max_iterations,
        int max_effort)
{
    {
        ofstream judgment_file("judgments");
    }

    auto start = std::chrono::steady_clock::now();
    cerr<<"Loading document features on memory"<<endl;
    set_doc_features(doc_features_path);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);

    cerr<<"Read "<<doc_features.size()<<" docs in "<<duration.count()<<"ms"<<endl;
    int dimensionality = 0;
    for(auto &feature_vec: doc_features){
        for(auto feature: feature_vec.features_)
            dimensionality = max(dimensionality, feature.id_);
    }
    dimensionality++;

    vector<int> judgments;
    int cur_effort = 0;
    int is_bmi = (judgements_per_iteration == -1);
    if(is_bmi)
        judgements_per_iteration = 1;

    training_data = get_initial_training_data(seed_path);

    for(int cur_iteration = 0;;cur_iteration++){
        if((max_iterations != -1 && cur_iteration >= max_iterations) || (max_effort != -1 && cur_effort >= max_effort)){
            vector<int> results;
            results.push_back(-1);
            add_to_judgment_queue(results);
            return;
        }

        cerr<<"Beginning Iteration "<<cur_iteration<<endl;

        randomize_non_rel_docs(training_data);

        // Training
        start = std::chrono::steady_clock::now();

        SfWeightVector w(dimensionality);
        run_classifier(training_data, w, dimensionality);

        auto weights = w.AsFloatVector();
        duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Training finished in "<<duration.count()<<"ms"<<endl;

        sort(judgments.begin(), judgments.end());
        vector<int> results;

        // Scoring
        start = std::chrono::steady_clock::now();
        rescore_documents(weights, num_threads, judgements_per_iteration, judgments, results);
        duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Rescored "<<doc_features.size()<<" documents in "<<duration.count()<<"ms"<<endl;

        cur_effort += results.size();
        if(max_effort >= 0 && cur_effort >= max_effort){
            while(cur_effort > max_effort){
                results.pop_back();
                cur_effort--;
            }
        }
        add_to_judgment_queue(results);
        wait_for_judgments();
        judgments.insert(judgments.begin(), results.begin(), results.end());
        if(is_bmi){
            judgements_per_iteration += (judgements_per_iteration + 9)/10;
        }
    }
}
