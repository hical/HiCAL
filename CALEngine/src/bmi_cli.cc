#include<fstream>
#include<thread>
#include "simple-cmd-line-helper.h"
#include "bmi.h"

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
};

int get_judgment_qrel(string topic_id, string doc_id){
    static Qrel qrel(CMD_LINE_STRINGS["--qrel"]);
    return qrel.get_judgment(topic_id, doc_id);
}

int main(int argc, char **argv){
    AddFlag("--doc-features", "Path of the file with list of document features", string(""));
    AddFlag("--query-features", "Path of the file with query features", string(""));
    AddFlag("--judgments-per-iteration", "Number of docs to judge per iteration (-1 for BMI default)", int(-1));
    AddFlag("--num-iterations", "Set max number of training iterations", int(-1));
    AddFlag("--max-effort", "Set max effort", int(-1));
    AddFlag("--qrel", "Use the qrel file for judgment", string(""));
    AddFlag("--topic-id", "Topic id for parsing qrel", string(""));
    AddFlag("--threads", "Number of threads to use for scoring", int(8));
    AddFlag("--async-mode", "Enable greedy async mode for classifier and rescorer, overrides --judgment-per-iteration", bool(false));
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

    if(CMD_LINE_STRINGS["--query-features"].length() == 0){
        cerr<<"Required argument --query-features missing"<<endl;
        return -1;
    }

    if(CMD_LINE_STRINGS["--qrel"].length() > 0){
        if(CMD_LINE_STRINGS["--topic-id"].length() == 0){
            cerr<<"Required argument --topic-id when --qrel is given"<<endl;
            return -1;
        }
    }

    auto t = thread(run_bmi,
            CMD_LINE_STRINGS["--doc-features"],
            CMD_LINE_STRINGS["--query-features"],
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--num-iterations"],
            CMD_LINE_INTS["--max-effort"]);

    auto get_judgment = get_judgment_stdin;
    if(CMD_LINE_STRINGS["--qrel"] != ""){
        get_judgment = get_judgment_qrel;
    }

    ofstream logfile(CMD_LINE_STRINGS["--judgment-logpath"]);
    string doc_id;
    while((doc_id = get_doc_to_judge()) != ""){
        int judgment = get_judgment(CMD_LINE_STRINGS["--topic-id"], doc_id);
        record_judgment(doc_id, judgment);
        logfile << doc_id <<" "<< (judgment == -1?0:judgment)<<endl;
    }
    logfile.close();
    t.join();
}
