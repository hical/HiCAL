#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include "scorer.h"
#include "sofiaml/sofia-ml-methods.h"
#include "sofiaml/sf-data-set.h"
#include "simple-cmd-line-helper.h"

using namespace std;

std::mt19937 rand_generator;
void train(vector<pair<int, int>> judgments, Scorer *scorer, SfWeightVector &w){
    vector<const SfSparseVector*> positives, negatives;
    // Sampling random non_rel documents
    uniform_int_distribution<int> distribution(0, scorer->doc_features.size()-1);
    for(int i = 1;i<=100;i++){
        int idx = distribution(rand_generator);
        negatives.push_back(&scorer->doc_features[idx]);
    }

    for(pair<int, int> judgment: judgments){
        if(judgment.second > 0)
            positives.push_back(&scorer->doc_features[judgment.first]);
        else
            negatives.push_back(&scorer->doc_features[judgment.first]);
    }
    
    sofia_ml::StochasticRocLoop(positives,
            negatives,
            sofia_ml::LOGREG_PEGASOS,
            sofia_ml::PEGASOS_ETA,
            0.0001,
            10000000.0,
            200000,
            &w);
}

int RUNNING = 0;
mutex running_mutex;
void job(string training, string out, Scorer *scorer){
    vector<pair<int, int>> judgments;
    ifstream fin(training);
    string doc_id;
    int val;
    while(fin>>val>>doc_id){
        judgments.push_back({scorer->doc_ids_inv_map[doc_id], val});
    }
    fin.close();
    SfWeightVector w(scorer->dimensionality);
    if(judgments.size() == 0){
        cerr<<"No training examples found for "<<training<<endl;
    }else{
        train(judgments, scorer, w);
    }

    auto weights = w.AsFloatVector();
    // Scoring
    vector<pair<int, float>> results = scorer->rescore_all_documents(weights, 1);

    ofstream fout(out);
    for(pair<int, float> result: results){
        if(result.first >= 0 && result.first < scorer->doc_features.size())
            fout<<scorer->doc_features[result.first].doc_id<<" "<<result.second<<endl;
    }
    fout.close();
    cerr<<"Finished ranking for "<<training<<endl;
    {
        lock_guard<mutex> lock(running_mutex);
        RUNNING--;
    }
}

int main(int argc, char **argv){
    AddFlag("--doc-features", "Path of the file with list of document features", string(""));
    AddFlag("--threads", "Number of threads", int(4));
    /* AddFlag("--training", "Number of docs to judge per iteration (-1 for BMI default)", int(-1)); */
    /* AddFlag("--out", "Number of threads to use for scoring", int(8)); */
    AddFlag("--help", "Show Help", bool(false));

    ParseFlags(argc, argv);

    if(CMD_LINE_BOOLS["--help"]){
        ShowHelp();
        return 0;
    }

    if(CMD_LINE_STRINGS["--doc-features"].length() == 0){
        cerr<<"Required argument --doc-features missing"<<endl;
        return -1;
    }

    /* if(CMD_LINE_STRINGS["--training"].length() == 0){ */
    /*     cerr<<"Required argument --training missing"<<endl; */
    /*     return -1; */
    /* } */

    /* if(CMD_LINE_STRINGS["--out"].length() == 0){ */
    /*     cerr<<"Required argument --out missing"<<endl; */
    /*     return -1; */
    /* } */

    // Load docs
    auto start = std::chrono::steady_clock::now();
    cerr<<"Loading document features on memory"<<endl;
    Scorer scorer(CMD_LINE_STRINGS["--doc-features"]);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Read "<<scorer.doc_features.size()<<" docs in "<<duration.count()<<"ms"<<endl;
    string training, out;
    while(getline(cin, training)){
        if(training.length() == 0){
            cerr<<"Error: blank line"<<endl;
            return 1;
        }
        getline(cin, out);
        if(out.length() == 0){
            cerr<<"Error: blank line"<<endl;
            return 1;
        }
        while(RUNNING >= CMD_LINE_INTS["--threads"]){
            this_thread::sleep_for(chrono::milliseconds(200));
        }
        {
            lock_guard<mutex> lock(running_mutex);
            RUNNING++;
        }
        auto t = thread(job, training, out, &scorer);
        t.detach();
    }
    while(RUNNING > 0){
        this_thread::sleep_for(chrono::milliseconds(200));
    }
}
