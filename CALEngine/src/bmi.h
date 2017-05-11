#ifndef BMI_H
#define BMI_H

#include<random>
#include<mutex>
#include<set>
#include "scorer.h"
#include "sofiaml/sofia-ml-methods.h"
#include "sofiaml/sf-data-set.h"

class BMI{
    private:

    Scorer *scorer;

    // Number of threads used for rescoring docs
    int num_threads;

    // Number of judgments to do before training weights
    int judgments_per_iteration;

    // Maximum effort allowed before exiting
    int max_effort;

    // Maximum number of training iterations allowed before exiting
    int max_iterations;

    // If true, the judgments_per_iteration grows with every iteration
    bool is_bmi;

    // The current state of CAL
    struct{
        int cur_iteration = 0;
        int cur_num_judgments = 0;
    }state;

    // Stores an ordered list of documents to judge based on the classifier scores
    std::vector<int> judgment_list;

    // A set of document ids which have already been judged
    std::set<int> finished_judgments;

    // rand() shouldn't be used because it is not thread safe
    std::mt19937 rand_generator;

    // Current of dataset being used to train the classifier
    SfDataSet training_data;

    // Whenever judgements are received, they are put into training_cache,
    // to prevent any race condition in case training_data is being used by the
    // classifier
    std::unordered_map<int, int> training_cache;

    // Mutexes to control access to certain objects
    std::mutex judgment_list_mutex;
    std::mutex finished_judgments_mutex;
    std::mutex training_cache_mutex;

    // samples 100 documents from the corpus to be used as non relevant examples
    // for training. NOTE: current method is hacky and assumes indices 1-100 in
    // the training_data.features_ to be the random non-rel docs
    void randomize_non_rel_docs();

    // Initializes training_data with seed query
    static SfDataSet get_initial_training_data(const SfSparseVector &seed);

    // train using the current training set and assign the weights to `w`
    void train(SfWeightVector &w);

    // Add the ids to the judgment list
    void add_to_judgment_list(const std::vector<int> &ids);

    // blocks the thread until all the judgments are done
    void wait_for_judgments();

    // Handler for performing an iteration
    void perform_iteration();

    // Handler for performing a training iteration
    std::vector<int> perform_training_iteration();

    public:
    BMI(const SfSparseVector &seed,
        Scorer *scorer,
        int num_threads,
        int judgments_per_iteration,
        int max_effort,
        int max_iterations);

    // Get upto `count` number of documents from `judgment_list`
    std::vector<std::string> get_doc_to_judge(int count);

    // Record judgment (-1 or 1) for a given doc_id
    void record_judgment(std::string doc_id, int judgment);

    // Begin CAL
    void run();
};

#endif // BMI_H
