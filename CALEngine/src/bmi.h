#include<iostream>
#include<random>
#include<chrono>
#include<algorithm>
#include<queue>
#include<mutex>
#include<chrono>
#include<thread>
#include<unordered_set>
#include<cassert>
#include<set>
#include "scorer.h"
#include "sofiaml/sofia-ml-methods.h"
#include "sofiaml/sf-data-set.h"

using namespace std;

class BMI{
    private:

    // Number of threads used for rescoring docs
    int num_threads;
    int judgments_per_iteration;
    int max_effort;
    int max_iterations;
    // Stores an ordered list of documents to judge based on the classifier scores
    vector<int> judgment_list;
    // A set of document ids which have already been judged
    set<int> finished_judgments;

    mt19937 rand_generator;

    // Current of dataset being used to train the classifier
    SfDataSet training_data;
    // Whenever judgements are received, they are put into training_cache,
    // to prevent any race condition in case training_data is being used by the
    // classifier
    unordered_map<int, int> training_cache;

    // Mutexes to control access to above objects
    mutex judgment_list_mutex;
    mutex finished_judgments_mutex;
    mutex training_cache_mutex;

    // samples 100 documents from the corpus to be used as non relevant examples
    // for training. NOTE: current method is hacky and assumes indices 1-100 in
    // the training_data.features_ to be the random non-rel docs
    void randomize_non_rel_docs();
    // Initializes training_data with seed query
    static SfDataSet get_initial_training_data(const SfSparseVector &seed);
    void train(SfWeightVector &w);
    void add_to_judgment_list(const vector<int> &ids);
    void wait_for_judgments();
    vector<int> perform_training_iteration();

    public:
    BMI(const SfSparseVector &seed,
        int num_threads,
        int judgments_per_iteration,
        int max_effort,
        int max_iterations);
    vector<string> get_doc_to_judge(int count);
    void record_judgment(string doc_id, int judgment);
    void run();
};

BMI::BMI(const SfSparseVector &seed,
        int num_threads,
        int judgments_per_iteration,
        int max_effort,
        int max_iterations)
    :training_data(get_initial_training_data(seed)),
    num_threads(num_threads),
    judgments_per_iteration(judgments_per_iteration),
    max_effort(max_effort),
    max_iterations(max_iterations)
{
}

void BMI::randomize_non_rel_docs(){
    uniform_int_distribution<int> distribution(0, doc_features.size()-1);
    for(int i = 1;i<=100;i++){
        int idx = distribution(rand_generator);
        if(training_data.NumExamples() < i+1)
            training_data.AddLabeledVector(doc_features[idx], -1);
        else
            training_data.ModifyLabeledVector(i, doc_features[idx], -1);
    }
}

SfDataSet BMI::get_initial_training_data(const SfSparseVector &seed){
    // Todo generalize seed docs (support multiple rel/non-rel seeds)
    SfDataSet training_data = SfDataSet(true);
    training_data.AddLabeledVector(seed, 1);
    return training_data;
}

void BMI::train(SfWeightVector &w){
    sofia_ml::StochasticRocLoop(training_data,
            sofia_ml::LOGREG_PEGASOS,
            sofia_ml::PEGASOS_ETA,
            0.0001,
            10000000.0,
            200000,
            &w);
}

vector<string> BMI::get_doc_to_judge(int count=1){
    while(true){
        {
            lock_guard<mutex> lock(judgment_list_mutex);
            lock_guard<mutex> lock2(training_cache_mutex);
            lock_guard<mutex> lock3(finished_judgments_mutex);
            if(judgment_list.size() > 0){
                vector<string> ret;
                for(int id: judgment_list){
                    if(id == -1 || ret.size() >= count)
                        break;
                    ret.push_back(doc_features[id].doc_id);
                }
                return ret;
            }
            /* for(int id: judgment_list){ */
            /*     // poison value */
            /*     if(id == -1){ */
            /*         return {}; */
            /*     } */
            /*     if(finished_judgments.find(id) == finished_judgments.end() \ */
            /*             && training_cache.find(id) == training_cache.end()) */
            /*         return doc_features[id].doc_id; */
            /* } */
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
            if(judgment_list.size() == 0)
                return;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void BMI::record_judgment(string doc_id, int judgment){
    int id = doc_ids_inv_map[doc_id];
    {
        lock_guard<mutex> lock(training_cache_mutex);
        training_cache[id] = judgment;
    }
    {
        lock_guard<mutex> lock(judgment_list_mutex);
        auto it = std::find(judgment_list.begin(), judgment_list.end(), id);
        if(it != judgment_list.end())
            judgment_list.erase(it);
    }
}

vector<int> BMI::perform_training_iteration(){
    randomize_non_rel_docs();

    {
        lock_guard<mutex> lock(training_cache_mutex);
        lock_guard<mutex> lock2(finished_judgments_mutex);
        for(pair<int, int> training: training_cache){
            training_data.AddLabeledVector(doc_features[training.first], training.second);
            finished_judgments.insert(training.first);
        }
        training_cache.clear();
    }

    // Training
    auto start = std::chrono::steady_clock::now();

    SfWeightVector w(dimensionality);
    train(w);

    auto weights = w.AsFloatVector();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Training finished in "<<duration.count()<<"ms"<<endl;

    vector<int> results;

    // Scoring
    start = std::chrono::steady_clock::now();
    rescore_documents(weights, num_threads, judgments_per_iteration, finished_judgments, results);
    duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Rescored "<<doc_features.size()<<" documents in "<<duration.count()<<"ms"<<endl;

    return results;
}

void BMI::run()
{
    int is_bmi = (judgments_per_iteration == -1);
    if(is_bmi)
        judgments_per_iteration = 1;

    for(int cur_iteration = 0;;cur_iteration++){
        if(max_iterations != -1 && cur_iteration >= max_iterations){
            vector<int> results;
            results.push_back(-1);
            add_to_judgment_list(results);
        }

        {
            lock_guard<mutex> lock(judgment_list_mutex);
            if(judgment_list.size() > 0 && judgment_list[0] == -1)
                return;
        }

        cerr<<"Beginning Iteration "<<cur_iteration<<endl;
        auto results = perform_training_iteration();
        add_to_judgment_list(results);
        wait_for_judgments();

        if(is_bmi){
            judgments_per_iteration += (judgments_per_iteration + 9)/10;
        }
    }
}
