#include<iostream>
#include<chrono>
#include<algorithm>
#include<fstream>
#include<cstdlib>
#include "scorer.h"
using namespace std;

int JUDGEMENTS_PER_TRAINING = 1;

string get_feature_string(SparseFeatureVector &sfv, int judgment){
    string ret = to_string(judgment);
    for(auto feature: sfv){
        ret += " " + to_string(feature.id) + ":" + to_string(feature.weight);
    }
    return ret;
}
void generate_random(){
    ofstream fout("nonrel.random");
    for(int i = 0;i<100;i++){
        int idx = rand() % doc_features.size();
        fout<<get_feature_string(doc_features[idx], -1)<<"\n";
    }
}

void record_judgment(char choice, int id){
    ofstream fout("judgments", ios_base::app | ios_base::out);
    int rel = -1;
    if(choice == 'y')
        rel = 1;
    fout<<get_feature_string(doc_features[id], rel)<<"\n";
}

void initialize_training(string fname){
    generate_random();
    string command = "cat " + fname + " nonrel.random judgments > training";
    system(command.c_str());
}

void run_classifier(int dimensionality){
    cerr<<"Running Classifier"<<endl;
    string command = "bash -c \"./sofia-ml --learner_type logreg-pegasos --loop_type roc --lambda 0.0001";
    command += " --iterations 200000 --training_file training --dimensionality "+to_string(dimensionality)+" --model_out svm_model &>> training.log\"";
    system(command.c_str());
}

int main(int argc, char *argv[]){
    {
        ofstream judgment_file("judgments");
    }

    auto start = std::chrono::steady_clock::now();
    cerr<<"Loading document features on memory"<<endl;
    set_doc_features(argv[1]);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);

    cerr<<"Read "<<doc_features.size()<<" docs in "<<duration.count()<<"ms"<<endl;
    int dimensionality = 0;
    for(auto &feature_vec: doc_features){
        for(auto feature: feature_vec)
            dimensionality = max(dimensionality, feature.id);
    }
    dimensionality++;

    vector<int> judgments;
    while(1){
        initialize_training(argv[2]);

        // Training
        start = std::chrono::steady_clock::now();
        run_classifier(dimensionality);
        auto weights = parse_model_file("svm_model");
        duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Training finished in "<<duration.count()<<"ms"<<endl;

        sort(judgments.begin(), judgments.end());
        vector<int> results;

        // Scoring
        start = std::chrono::steady_clock::now();
        rescore_documents(weights, 8, JUDGEMENTS_PER_TRAINING, judgments, results);
        duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Rescored "<<doc_features.size()<<" documents in "<<duration.count()<<"ms"<<endl;

        cout<<"Judge "<<results[0]<<" (y/n)"<<": ";

        // Judgment
        string command = "cp ./nyt/nyt_corpus/flat_data/" + doc_ids[results[0]] + ".xml ./preview.html";
        system(command.c_str());
        system("killall lynx");
        char ch;
        cin>>ch;
        record_judgment(ch, results[0]);
        judgments.push_back(results[0]);
    }
}
