#include <iostream>
#include <chrono>
#include <algorithm>
#include "scorer.h"
using namespace std;

int main(int argc, char *argv[]){
    auto weights = parse_model_file(argv[1]);
    cerr<<weights.size()<<endl;

    auto start = std::chrono::steady_clock::now();
    cerr<<"Loading document features on memory"<<endl;
    set_doc_features(argv[2]);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);

    cerr<<"Read "<<doc_features.size()<<" docs in "<<duration.count()<<"ms"<<endl;

    float avg = 0.0;
    for(int i = 0;i<10;i++){
        vector<int> results;
        vector<int> judgments;
        for(int j = 0;j<i*500;j++){
            judgments.push_back(rand() % doc_features.size());
        }
        sort(judgments.begin(), judgments.end());
        start = std::chrono::steady_clock::now();
        rescore_documents(weights, 1, 1, judgments, results);
        duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Rescored "<<doc_features.size()<<" documents in "<<duration.count()<<"ms"<<endl;
        avg += duration.count();
    }
    cerr<<"Average Rescoring Time: "<<avg/10<<endl;
}
