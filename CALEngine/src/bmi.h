#ifndef BMI_H
#define BMI_H

#include <random>
#include <mutex>
#include <set>
#include <map>
#include "dataset.h"

class BMI{
    protected:

    Dataset *documents;

    // Number of threads used for rescoring docs
    int num_threads;

    // Number of judgments to do before training weights
    int judgments_per_iteration;

    int training_iterations;

    int extra_judgment_docs = 50;

    // Async Mode
    bool async_mode;

    // If true, the judgments_per_iteration grows with every iteration
    bool is_bmi;

    // The current state of CAL
    struct State{
        uint32_t cur_iteration = 0;
        uint32_t next_iteration_target = 0;
        bool finished = false;
    }state;

    // Stores an ordered list of documents to judge based on the classifier scores
    std::vector<int> judgment_queue;

    // rand() shouldn't be used because it is not thread safe
    std::mt19937 rand_generator;

    // Current of dataset being used to train the classifier
    const Seed seed;
    std::map<int, int> judgments;
    vector<const SfSparseVector*> positives, negatives;
    int random_negatives_index;
    int random_negatives_size = 100;

    // Whenever judgements are received, they are put into training_cache,
    // to prevent any race condition in case training_data is being used by the
    // classifier
    std::unordered_map<int, int> training_cache;

    // Mutexes to control access to certain objects
    std::mutex judgment_list_mutex;
    std::mutex training_mutex;
    std::mutex async_training_mutex;
    std::mutex training_cache_mutex;
    std::mutex state_mutex;

    // Tasks to perform in order to finish the session
    void finish_session();
    bool try_finish_session();

    // train using the current training set and assign the weights to `w`
    virtual vector<float> train();

    // Add the ids to the judgment list
    void add_to_judgment_list(const std::vector<int> &ids);

    // Add to training_cache
    void add_to_training_cache(int id, int judgment);

    // Handler for performing an iteration
    void perform_iteration();
    void perform_iteration_async();
    void sync_training_cache();

    public:
    BMI(Seed seed,
        Dataset *documents,
        int num_threads,
        int judgments_per_iteration,
        bool async_mode,
        int training_iterations,
        bool initialize = true);

    // Handler for performing a training iteration
    virtual std::vector<int> perform_training_iteration();

    // Check if a given document is judged, lock training_cache_mutex before using this (not done here for efficiency purpose)
    virtual bool is_judged(int id) {
        return judgments.find(id) != judgments.end() || training_cache.find(id) != training_cache.end();
    }

    // Get upto `count` number of documents from `judgment_list`
    virtual std::vector<std::string> get_doc_to_judge(uint32_t count);

    // Record judgment (-1 or 1) for a given doc_id
    virtual void record_judgment(std::string doc_id, int judgment);

    // Record batch judgments
    virtual void record_judgment_batch(std::vector<std::pair<std::string, int>> judgments);

    // Get ranklist for current classifier state
    virtual vector<std::pair<string, float>> get_ranklist();

    virtual Dataset *get_ranking_dataset() {return documents;};
    virtual string get_log() {return "{}";}
    Dataset *get_dataset() {return documents;};

    struct State get_state(){
        return state;
    }
};

#endif // BMI_H
