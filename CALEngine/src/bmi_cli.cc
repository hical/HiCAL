#include <iostream>
#include <fstream>
#include <thread>
#include <ctime>
#include <climits>
#include "utils/utils.h"
#include "utils/simple-cmd-line-helper.h"
#include "bmi_para.h"
#include "bmi_reduced_ranking.h"
#include "bmi_online_learning.h"
#include "bmi_precision_delay.h"
#include "bmi_recency_weighting.h"
#include "bmi_forget.h"
#include "features.h"
#include "utils/feature_parser.h"

using namespace std;

vector<string> BMI_TYPES = {
    "BMI_DOC",
    "BMI_PARA",
    "BMI_PARTIAL_RANKING",
    "BMI_ONLINE_LEARNING",
    "BMI_PRECISION_DELAY",
    "BMI_RECENCY_WEIGHTING",
    "BMI_FORGET"
};

int get_judgment_stdin(string topic_id, string doc_id){
    int rel;
    cout<<"Judge "<<doc_id<<": ";
    cin>>rel;
    return rel;
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

    int get_recall(string topic) {
        int recall = 0;
        for(auto &key_val: qrel_map){
            if(key_val.first.first == topic && key_val.second > 0)
                recall += 1;
        }
        return recall;
    }
}qrel;

int get_judgment_qrel(string topic_id, string doc_id){
    doc_id = doc_id.substr(0, doc_id.find("."));
    topic_id = topic_id.substr(0, topic_id.find("."));
    return qrel.get_judgment(topic_id, doc_id);
}

map<string, Seed> generate_seed_queries(string fname, const Dataset &dataset){
    map<string, Seed> seeds;
    ifstream fin(fname);
    string topic_id, query;
    int rel;
    while(fin>>topic_id>>rel){
        getline(fin, query);
        seeds[topic_id].push_back({features::get_features(query, dataset), rel});
    }

    return seeds;
}

void begin_bmi_helper(const pair<string, Seed> &seed_query, const unique_ptr<Dataset> &documents, const unique_ptr<ParagraphDataset> &paragraphs){
    cerr<<"Topic "<<seed_query.first<<endl;
    unique_ptr<BMI> bmi;
    const string &mode = CMD_LINE_STRINGS["--mode"];
    if(mode == "BMI_DOC"){
        bmi = make_unique<BMI>(seed_query.second,
            documents.get(),
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--async-mode"],
            CMD_LINE_INTS["--training-iterations"]);
    } else if(mode == "BMI_PARA"){
        bmi = make_unique<BMI_para>(seed_query.second,
            documents.get(),
            paragraphs.get(),
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--async-mode"],
            CMD_LINE_INTS["--training-iterations"]);
    } else if(mode == "BMI_PARTIAL_RANKING"){
        bmi = make_unique<BMI_reduced_ranking>(seed_query.second,
            documents.get(),
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--async-mode"],
            CMD_LINE_INTS["--partial-ranking-subset-size"], CMD_LINE_INTS["--partial-ranking-refresh-period"],
            CMD_LINE_INTS["--training-iterations"]);
    } else if(mode == "BMI_ONLINE_LEARNING"){
        bmi = make_unique<BMI_online_learning>(seed_query.second,
            documents.get(),
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--async-mode"],
            CMD_LINE_INTS["--online-learning-refresh-period"],
            CMD_LINE_FLOATS["--online-learning-delta"],
            CMD_LINE_INTS["--training-iterations"]);
    } else if(mode == "BMI_PRECISION_DELAY"){
        bmi = make_unique<BMI_precision_delay>(seed_query.second,
            documents.get(),
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--async-mode"],
            CMD_LINE_FLOATS["--precision-delay-threshold"],
            CMD_LINE_INTS["--precision-delay-window"],
            CMD_LINE_INTS["--training-iterations"]);
    } else if(mode == "BMI_RECENCY_WEIGHTING"){
        bmi = make_unique<BMI_recency_weighting>(seed_query.second,
            documents.get(),
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--async-mode"],
            CMD_LINE_FLOATS["--recency-weighting-param"],
            CMD_LINE_INTS["--training-iterations"]);
    } else if(mode == "BMI_FORGET"){
        bmi = make_unique<BMI_forget>(seed_query.second,
            documents.get(),
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--async-mode"],
            CMD_LINE_INTS["--forget-remember-count"],
            CMD_LINE_INTS["--forget-refresh-period"],
            CMD_LINE_INTS["--training-iterations"]);
    } else {
        cerr<<"Invalid bmi_type"<<endl;
        return;
    }

    auto get_judgment = get_judgment_stdin;
    if(CMD_LINE_STRINGS["--qrel"] != ""){
        get_judgment = get_judgment_qrel;
    }

    if(CMD_LINE_BOOLS["--generate-ranklists"]){
        string ranklist_path = CMD_LINE_STRINGS["--judgment-logpath"] + "/" + seed_query.first;
        ofstream logfile(ranklist_path);
        vector<pair<string, int>> seed_judgments;
        for(auto &entry: qrel.qrel_map) {
            if(entry.first.first == seed_query.first){
                auto &doc_id = entry.first.second;
                if(bmi->get_dataset()->get_index(doc_id) == bmi->get_dataset()->NPOS) {
                    std::cerr << "Document " << doc_id << " not found in corpus (aborting...)" << std::endl;
                    exit(1);
                }
                seed_judgments.push_back({doc_id, entry.second});
            }
        }
        bmi->record_judgment_batch(seed_judgments);
        for(auto &entry: bmi->get_ranklist()){
            logfile << entry.first << " " << entry.second << "\n";
        }
        logfile.close();
        cerr << "Generated ranklist at: " << ranklist_path << endl;;
        return;
    }

    vector<string> doc_ids;
    int max_effort = CMD_LINE_INTS["--max-effort"];
    int max_iterations = CMD_LINE_INTS["--num-iterations"];

    if(max_effort <= 0){
        float max_effort_factor = CMD_LINE_FLOATS["--max-effort-factor"];
        if(max_effort_factor > 0)
            max_effort = qrel.get_recall(seed_query.first) * max_effort_factor;
        else
            max_effort = INT_MAX;
    }
    if(max_iterations < 0)
        max_iterations = INT_MAX;

    int effort = 0;
    ofstream logfile(CMD_LINE_STRINGS["--judgment-logpath"] + "/" + seed_query.first);
    while(!(doc_ids = bmi->get_doc_to_judge(1)).empty()){
        int judgment = get_judgment(seed_query.first, doc_ids[0]);
        bmi->record_judgment(doc_ids[0], judgment);
        logfile << doc_ids[0] <<" "<< (judgment == -1?0:judgment)<<endl;
        effort++;
        if(effort >= max_effort || bmi->get_state().cur_iteration >= max_iterations)
            break;
    }
    logfile.close();
}

void SanityCheck(){
    if(CMD_LINE_BOOLS["--help"]){
        ShowHelp();
        exit(0);
    }

    const string &mode = CMD_LINE_STRINGS["--mode"];
    if(find(BMI_TYPES.begin(), BMI_TYPES.end(), mode) == BMI_TYPES.end()){
        cerr<<"Invalid --mode "+mode<<endl;
        exit(1);
    }

    if(CMD_LINE_STRINGS["--doc-features"].length() == 0){
        cerr<<"--doc-features missing"<<endl;
        exit(1);
    }

    if(CMD_LINE_STRINGS["--query"].length() == 0){
        cerr<<"--query missing"<<endl;
        exit(1);
    }

    if(CMD_LINE_INTS["--training-iterations"] < 0){
        cerr<<"non-negative --training-iterations required"<<endl;
        exit(1);
    }

    if(mode == "BMI_FORGET"){
        if(CMD_LINE_INTS["--forget-remember-count"] < 0){
            cerr<<"non-negative --forget-remember-count required"<<endl;
            exit(1);
        }
    } else if (mode == "BMI_PARA") {
        if(CMD_LINE_STRINGS["--para-features"] == ""){
            cerr<<"--para-features required"<<endl;
            exit(1);
        }
    } else if (mode == "BMI_PRECISION_DELAY") {
        if(CMD_LINE_INTS["--precision-delay-window"] < 1){
            cerr<<"positive --precision-delay-window required"<<endl;
            exit(1);
        }
    } else if (mode == "BMI_RECENCY_WEIGHTING") {
        if(CMD_LINE_FLOATS["--recency-weighting-param"] < 1){
            cerr<<"--recency-weighting-param >= 1 required"<<endl;
            exit(1);
        }
    } else if (mode == "BMI_PARTIAL_RANKING") {
        if(CMD_LINE_INTS["--partial-ranking-subset-size"] <= 0){
            cerr<<"positive --partial-ranking-subset-size required"<<endl;
            exit(1);
        }
        if(CMD_LINE_INTS["--partial-ranking-refresh-period"] <= 0){
            cerr<<"positive --partial-ranking-refresh-period required"<<endl;
            exit(1);
        }
        if(CMD_LINE_INTS["--partial-ranking-subset-size"] < CMD_LINE_INTS["--partial-ranking-refresh-period"]){
            cerr<<"--partial-ranking-subset-size should be greater or equal to --partial-ranking-refresh-period"<<endl;
            exit(1);
        }
    } else if (mode == "BMI_ONLINE_LEARNING") {
        if(CMD_LINE_INTS["--online-learning-refresh-period"] < 0) {
            cerr<<"non-negative --online-learning-refresh-period required"<<endl;
            exit(1);
        }
    }

}

string get_help_mode_string(){
    string ret = "Set strategy: (default) ";
    for(string &mode: BMI_TYPES){
        ret += mode + ", ";
    }
    return ret.substr(0, ret.size() - 2);
}

int main(int argc, char **argv){
    TIMER_BEGIN(BMI_CLI);
    AddFlag("--mode", get_help_mode_string().c_str(), string("BMI_DOC"));
    AddFlag("--doc-features", "Path of the file with list of document features", string(""));
    AddFlag("--para-features", "Path of the file with list of paragraph features (BMI_PARA)", string(""));
    AddFlag("--query", string("Path of the file with queries (one seed per line; each line is <topic_id> <rel> <string>; can have multiple seeds per topic)"), string(""));
    AddFlag("--judgments-per-iteration", "Number of docs to judge per iteration (-1 for BMI default)", int(-1));
    AddFlag("--num-iterations", "Set max number of refresh iterations", int(-1));
    AddFlag("--max-effort", "Set max effort (number of judgments)", int(-1));
    AddFlag("--max-effort-factor", "Set max effort as a factor of recall", float(-1));
    AddFlag("--training-iterations", "Set number of training iterations", int(200000));
    AddFlag("--partial-ranking-subset-size", "Set subset size for partial ranking (BMI_PARTIAL_RANKING)", int(0));
    AddFlag("--partial-ranking-refresh-period", "Set refresh period for partial ranking (BMI_PARTIAL_RANKING)", int(0));
    AddFlag("--online-learning-refresh-period", "Set refresh period for online learning (BMI_ONLINE_LEARNING)", int(0));
    AddFlag("--online-learning-delta", "Set delta for online learning (BMI_ONLINE_LEARNING)", float(0.002));
    AddFlag("--precision-delay-threshold", "Set threshold for precision delay (BMI_PRECISION_DELAY)", float(0));
    AddFlag("--precision-delay-window", "Set window size for precision delay (BMI_PRECISION_DELAY)", int(10));
    AddFlag("--recency-weighting-param", "Set parameter for recency weighting (BMI_RECENCY_WEIGHTING)", float(-1));
    AddFlag("--forget-remember-count", "Number of documents to remember (BMI_FORGET)", int(-1));
    AddFlag("--forget-refresh-period", "Period for full training (BMI_FORGET)", int(-1));
    AddFlag("--qrel", "Qrel file to use for judgment", string(""));
    AddFlag("--threads", "Number of threads to use for scoring", int(8));
    AddFlag("--jobs", "Number of concurrent jobs (topics)", int(1));
    AddFlag("--async-mode", "Enable greedy async mode for classifier and rescorer, overrides --judgment-per-iteration and --num-iterations", bool(false));
    AddFlag("--generate-ranklists", "Generate ranklists instead of simulating", bool(false));
    AddFlag("--judgment-logpath", "Path to log judgments. Specify a directory within which topic-specific logs will be generated.", string("./judgments.list"));
    AddFlag("--df", "Path of the file with list of terms and their document frequencies. The file contains space-separated word and df on every line. Specify only when df information is not encoded in the document features file.", string(""));
    AddFlag("--help", "Show Help", bool(false));

    ParseFlags(argc, argv);
    SanityCheck();

    // Load qrels
    if(CMD_LINE_STRINGS["--qrel"].length() > 0){
        qrel = Qrel(CMD_LINE_STRINGS["--qrel"]);
    }

    // Load docs
    unique_ptr<Dataset> documents = nullptr;
    unique_ptr<ParagraphDataset> paragraphs = nullptr;

    TIMER_BEGIN(documents_loader);
    cerr<<"Loading document features on memory"<<endl;
    {
        unique_ptr<FeatureParser> feature_parser;
        if(CMD_LINE_STRINGS["--df"].size() > 0)
            feature_parser = make_unique<BinFeatureParser>(CMD_LINE_STRINGS["--doc-features"], CMD_LINE_STRINGS["--df"]);
        else
            feature_parser = make_unique<BinFeatureParser>(CMD_LINE_STRINGS["--doc-features"]);
        documents = Dataset::build(feature_parser.get());
        cerr<<"Read "<<documents->size()<<" docs"<<endl;
    }
    TIMER_END(documents_loader);

    // Load para
    string para_features_path = CMD_LINE_STRINGS["--para-features"];
    if(para_features_path.length() > 0){
        TIMER_BEGIN(paragraph_loader);
        cerr<<"Loading paragraph features on memory"<<endl;
        {
            unique_ptr<FeatureParser> feature_parser;
            if(CMD_LINE_STRINGS["--df"].size() > 0)
                feature_parser = make_unique<BinFeatureParser>(para_features_path, "");
            else
                feature_parser = make_unique<BinFeatureParser>(para_features_path);
            paragraphs = ParagraphDataset::build(feature_parser.get(), *documents);
            cerr<<"Read "<<paragraphs->size()<<" paragraphs"<<endl;
        }
        TIMER_END(paragraph_loader);
    }

    // Load seed queries
    map<string, Seed> seeds = generate_seed_queries(CMD_LINE_STRINGS["--query"], *documents);

    // Start jobs
    // Todo: Better job management
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

    TIMER_END(BMI_CLI);
}
