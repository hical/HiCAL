#include <iostream>
#include <fstream>
#include <thread>
#include "utils/simple-cmd-line-helper.h"
#include "bmi_para.h"
#include "features.h"
#include "utils/feature_parser.h"

using namespace std;

int get_judgment_stdin(string topic_id, string doc_id){
    cout<<"Judge "<<doc_id<<" (y/n)"<<": ";
    string command = "cp ~/para/para/" + doc_id.substr(0, 4) + "/" + doc_id + " ./preview.html";
    system(command.c_str());
    system("killall lynx");
    char ch;
    cin>>ch;
    return (ch == 'y'?1:-1);
}

struct str_pair_hash {
    std::size_t operator () (const std::pair<string,string> &p) const {
        return std::hash<string>{}(p.first + "\n" + p.second);
    }
};

struct Qrel{
    unordered_map<pair<string, string>, int, str_pair_hash> qrel_map;
    Qrel(){}
    Qrel(string qrel_path){
        ifstream qrel_file(qrel_path);
        string topic, _, doc_id;
        int rel;
        while(qrel_file >> topic >> _ >> doc_id >> rel){
            qrel_map[{topic, doc_id}] = (rel == 0?-1:rel);
        }
        qrel_file.close();
    }

    int get_judgment(string topic, string doc_id){
        if(qrel_map.find({topic, doc_id}) == qrel_map.end())
            return -1;
        return qrel_map[{topic, doc_id}];
    }
}qrel;

int get_judgment_qrel(string topic_id, string doc_id){
    doc_id = doc_id.substr(0, doc_id.find("."));
    topic_id = topic_id.substr(0, topic_id.find("."));
    return qrel.get_judgment(topic_id, doc_id);
}

unordered_map<string, Seed> generate_seed_queries(string fname, int num_docs){
    unordered_map<string, Seed> seeds;
    ifstream fin(fname);
    string topic_id, query;
    int rel;
    while(fin>>topic_id>>rel){
        getline(fin, query);
        seeds[topic_id].push_back({features::get_features(query, num_docs), rel});
    }

    return seeds;
}

void begin_bmi_helper(const pair<string, Seed> &seed_query, const unique_ptr<Dataset> &documents, const unique_ptr<Dataset> &paragraphs){
    ofstream logfile(CMD_LINE_STRINGS["--judgment-logpath"] + "." + seed_query.first);
    cerr<<"Topic "<<seed_query.first<<endl;
    unique_ptr<BMI> bmi;
    if(paragraphs != nullptr){
        bmi = make_unique<BMI_para>(cref(seed_query.second),
            documents.get(),
            paragraphs.get(),
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--max-effort"],
            CMD_LINE_INTS["--num-iterations"],
            CMD_LINE_INTS["--async-mode"]);
    }else{
        bmi = make_unique<BMI>(cref(seed_query.second),
            documents.get(),
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--max-effort"],
            CMD_LINE_INTS["--num-iterations"],
            CMD_LINE_INTS["--async-mode"]);
    }

    auto get_judgment = get_judgment_stdin;
    if(CMD_LINE_STRINGS["--qrel"] != ""){
        get_judgment = get_judgment_qrel;
    }

    vector<string> doc_ids;
    while(!(doc_ids = bmi->get_doc_to_judge(1)).empty()){
        int judgment = get_judgment(seed_query.first, doc_ids[0]);
        bmi->record_judgment(doc_ids[0], judgment);
        logfile << doc_ids[0] <<" "<< (judgment == -1?0:judgment)<<endl;
    }
    logfile.close();
}

int main(int argc, char **argv){
    AddFlag("--doc-features", "Path of the file with list of document features", string(""));
    AddFlag("--para-features", "Path of the file with list of paragraph features", string(""));
    AddFlag("--df", "Path of the file with document frequency of each term", string(""));
    AddFlag("--query", string("Path of the file with queries (odd lines containing topic-id and even lines containing")+\
            string("respective query string)"), string(""));
    AddFlag("--judgments-per-iteration", "Number of docs to judge per iteration (-1 for BMI default)", int(-1));
    AddFlag("--num-iterations", "Set max number of training iterations", int(-1));
    AddFlag("--max-effort", "Set max effort", int(-1));
    AddFlag("--qrel", "Use the qrel file for judgment", string(""));
    AddFlag("--threads", "Number of threads to use for scoring", int(8));
    AddFlag("--jobs", "Number of concurrent jobs", int(1));
    AddFlag("--async-mode", "Enable greedy async mode for classifier and rescorer, overrides --judgment-per-iteration and --num-iterations", bool(false));
    AddFlag("--judgment-logpath", "Path to log judgments", string("./judgments.list"));
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

    if(CMD_LINE_STRINGS["--df"].length() == 0){
        cerr<<"Required argument --df missing"<<endl;
        return -1;
    }

    if(CMD_LINE_STRINGS["--query"].length() == 0){
        cerr<<"Required argument --query missing"<<endl;
        return -1;
    }

    if(CMD_LINE_STRINGS["--qrel"].length() > 0){
        qrel = Qrel(CMD_LINE_STRINGS["--qrel"]);
    }

    // Load docs
    unique_ptr<Dataset> documents = nullptr;
    unique_ptr<Dataset> paragraphs = nullptr;

    auto start = std::chrono::steady_clock::now();
    cerr<<"Loading document features on memory"<<endl;
    string doc_features_path = CMD_LINE_STRINGS["--doc-features"];
    documents = CAL::utils::BinFeatureParser(doc_features_path).get_all();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Read "<<documents->size()<<" docs in "<<duration.count()<<"ms"<<endl;

    // Load para
    string para_features_path = CMD_LINE_STRINGS["--para-features"];
    if(para_features_path.length() > 0){
        start = std::chrono::steady_clock::now();
        cerr<<"Loading paragraph features on memory"<<endl;
        paragraphs = CAL::utils::BinFeatureParser(para_features_path).get_all();
        duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Read "<<paragraphs->size()<<" docs in "<<duration.count()<<"ms"<<endl;
    }

    // Load queries
    features::init(CMD_LINE_STRINGS["--df"]);
    unordered_map<string, Seed> seeds = generate_seed_queries(CMD_LINE_STRINGS["--query"], documents->size());
    vector<thread> jobs;
    for(const pair<string, Seed> &seed_query: seeds){
        jobs.push_back(thread(begin_bmi_helper, seed_query, cref(documents), cref(paragraphs)));
        if(jobs.size() == CMD_LINE_INTS["--jobs"]){
            for(auto &t: jobs)
                t.join();
            jobs.clear();
        }
    }

    for(auto &t: jobs)
        t.join();
}
