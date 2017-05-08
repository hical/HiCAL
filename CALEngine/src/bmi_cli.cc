#include<fstream>
#include<thread>
#include "simple-cmd-line-helper.h"
#include "bmi.h"
#include "features.cc"

int get_judgment_stdin(string topic_id, string doc_id){
    cout<<"Judge "<<doc_id<<" (y/n)"<<": ";
    string command = "cp ./nyt/nyt_corpus/flat_data/" + doc_id + ".xml ./preview.html";
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
    return qrel.get_judgment(topic_id, doc_id);
}

vector<pair<string, SfSparseVector>> generate_seed_queries(string fname){
    vector<pair<string, SfSparseVector>> seed_queries;
    ifstream fin(fname);
    string topic_id, query;
    while(getline(fin, topic_id)){
        getline(fin, query);
        seed_queries.push_back({topic_id, get_features(query, doc_features.size())});
    }

    return seed_queries;
}

void begin_bmi_helper(pair<string, SfSparseVector> seed_query){
    ofstream logfile(CMD_LINE_STRINGS["--judgment-logpath"] + "." + seed_query.first);
    cerr<<"Topic "<<seed_query.first<<endl;
    BMI bmi(seed_query.second,
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--max-effort"],
            CMD_LINE_INTS["--num-iterations"]);
    auto t = thread(&BMI::run, &bmi);

    auto get_judgment = get_judgment_stdin;
    if(CMD_LINE_STRINGS["--qrel"] != ""){
        get_judgment = get_judgment_qrel;
    }

    string doc_id;
    while((doc_id = bmi.get_doc_to_judge()) != ""){
        int judgment = get_judgment(seed_query.first, doc_id);
        bmi.record_judgment(doc_id, judgment);
        logfile << seed_query.first <<" "<< doc_id <<" "<< (judgment == -1?0:judgment)<<endl;
    }
    t.join();
    logfile.close();
}

int main(int argc, char **argv){
    AddFlag("--doc-features", "Path of the file with list of document features", string(""));
    AddFlag("--df", "Path of the file with document frequency of each term", string(""));
    AddFlag("--query", "Path of the file with queries (odd lines containing topic-id and even lines containing \
        respective query string)", string(""));
    AddFlag("--judgments-per-iteration", "Number of docs to judge per iteration (-1 for BMI default)", int(-1));
    AddFlag("--num-iterations", "Set max number of training iterations", int(-1));
    AddFlag("--max-effort", "Set max effort", int(-1));
    AddFlag("--qrel", "Use the qrel file for judgment", string(""));
    AddFlag("--threads", "Number of threads to use for scoring", int(8));
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
    auto start = std::chrono::steady_clock::now();
    cerr<<"Loading document features on memory"<<endl;
    set_doc_features(CMD_LINE_STRINGS["--doc-features"]);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Read "<<doc_features.size()<<" docs in "<<duration.count()<<"ms"<<endl;

    // Load queries
    init(CMD_LINE_STRINGS["--df"]);
    vector<pair<string, SfSparseVector>> seed_queries = generate_seed_queries(CMD_LINE_STRINGS["--query"]);

    vector<thread> jobs;
    for(pair<string, SfSparseVector> seed_query: seed_queries){
        jobs.push_back(thread(begin_bmi_helper, ref(seed_query)));
        /* jobs.back().join(); */
    }
    for(auto &t: jobs)
        t.join();
}
