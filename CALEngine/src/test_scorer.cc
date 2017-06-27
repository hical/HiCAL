#include <iostream>
#include <chrono>
#include <algorithm>
#include "scorer.h"
#include "utils/feature_parser.h"
using namespace std;

int main(int argc, char *argv[]){

    auto start = std::chrono::steady_clock::now();
    cerr<<"Loading document features on memory"<<endl;
    auto features = CAL::utils::BinFeatureParser(argv[1]).get_all();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);

    cerr<<"Read "<<features->size()<<" docs in "<<duration.count()<<"ms"<<endl;

    Scorer scorer(features);

    float avg = 0.0;
    for(int i = 0;i<10;i++){
        vector<int> results;
        set<int> judgments;
        for(int j = 0;j<i*500;j++){
            judgments.insert(rand() % scorer.doc_features->size());
        }
        vector<float> weights(scorer.doc_features->size());
        for(float &wt: weights)
            wt = (float)rand()/(float)(RAND_MAX);

        start = std::chrono::steady_clock::now();
        scorer.rescore_documents(weights, 1, 10, judgments, results);
        duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Rescored "<<scorer.doc_features->size()<<" documents in "<<duration.count()<<"ms"<<endl;
        avg += duration.count();
    }
    cerr<<"Average Rescoring Time: "<<avg/10<<endl;
}
