#include<fstream>
#include<thread>
#include <fcgio.h>
#include "simple-cmd-line-helper.h"
#include "bmi.h"
#include "features.h"

using namespace std;
unordered_map<string, BMI*> SESSIONS;
unordered_map<string, thread> SESSION_THREADS;
Scorer *scorer;

// Get the uri without following and preceding slashes
string parse_action_from_uri(string uri){
    int st = 0, end = int(uri.length())-1;

    while(st < uri.length() && uri[st] == '/') st++;
    while(end >= 0 && uri[end] == '/') end--;

    return uri.substr(st, end - st + 1);
}

// Use a proper library someday
// returns a vector of <key, value> in given url query string
vector<pair<string, string>> parse_query_string(string query_string){
    vector<pair<string, string>> key_vals;
    string key, val;
    bool valmode = false;
    for(char ch: query_string){
        if(ch == '&'){
            if(key.length() > 0)
                key_vals.push_back({key, val});
            key = val = "";
            valmode = false;
        }else if(ch == '=' && !valmode){
            valmode = true;
        }else{
            if(valmode)
                val.push_back(ch);
            else
                key.push_back(ch);
        }
    }
    if(key.length() > 0)
        key_vals.push_back({key, val});
    return key_vals;
}

// returns the content field of request
// Built upon http://chriswu.me/blog/writing-hello-world-in-fcgi-with-c-plus-plus/
string read_content(const FCGX_Request & request){
    fcgi_streambuf cin_fcgi_streambuf(request.in);
    istream request_stream(&cin_fcgi_streambuf);

    char * content_length_str = FCGX_GetParam("CONTENT_LENGTH", request.envp);
    unsigned long content_length = 0;

    if (content_length_str) {
        content_length = atoi(content_length_str);
    }

    char * content_buffer = new char[content_length];
    request_stream.read(content_buffer, content_length);
    content_length = request_stream.gcount();

    do request_stream.ignore(1024); while (request_stream.gcount() == 1024);
    string content(content_buffer, content_length);
    delete [] content_buffer;
    return content;
}

// Given info write to the request's response
void write_response(const FCGX_Request & request, int status, string content_type, string content){
    fcgi_streambuf cout_fcgi_streambuf(request.out);
    ostream response_stream(&cout_fcgi_streambuf);
    response_stream << "Status: " << to_string(status) << "\r\n"
                    << "Content-type: " << content_type << "\r\n"
                    << "\r\n"
                    << content << "\n";
}

// Handler for API endpoint /begin
void begin_session_view(const FCGX_Request & request, const vector<pair<string, string>> &params){
    string session_id, query;

    for(auto kv: params){
        if(kv.first == "session_id"){
            session_id = kv.second;
        }else if(kv.first == "seed_query"){
            query = kv.second;
        }
    }

    if(SESSIONS.find(session_id) != SESSIONS.end()){
        write_response(request, 400, "application/json", "{\"error\": \"session already exists\"}");
        return;
    }

    if(session_id.size() == 0 || query.size() == 0){
        write_response(request, 400, "application/json", "{\"error\": \"Non empty session_id and query required\"}");
    }

    SESSIONS[session_id] = new BMI(features::get_features(query, scorer->doc_features.size()),
            scorer,
            CMD_LINE_INTS["--threads"],
            CMD_LINE_INTS["--judgments-per-iteration"],
            CMD_LINE_INTS["--max-effort"],
            CMD_LINE_INTS["--num-iterations"]);

     auto t = thread(&BMI::run, ref(*SESSIONS[session_id]));
     t.detach();

    // need proper json parsing!!
    write_response(request, 200, "application/json", "{\"session-id\": \""+session_id+"\"}");
}

// Fetch doc-ids in JSON
string get_docs(string session_id, int max_count){
    BMI *bmi = SESSIONS[session_id];
    vector<string> doc_ids = bmi->get_doc_to_judge(max_count);

    string doc_json = "[";
    for(string doc_id: doc_ids){
        if(doc_json.length() > 1)
            doc_json.push_back(',');
        doc_json += "\"" + doc_id + "\"";
    }
    doc_json.push_back(']');

    return "{\"session-id\": \"" + session_id + "\", \"docs\": " + doc_json + "}";
}

// Handler for /get_docs
void get_docs_view(const FCGX_Request & request, const vector<pair<string, string>> &params){
    string session_id;
    int max_count = 2;

    for(auto kv: params){
        if(kv.first == "session_id"){
            session_id = kv.second;
        }else if(kv.first == "max_count"){
            max_count = stoi(kv.second);
        }
    }

    if(session_id.size() == 0){
        write_response(request, 400, "application/json", "{\"error\": \"Non empty session_id required\"}");
    }

    if(SESSIONS.find(session_id) == SESSIONS.end()){
        write_response(request, 404, "application/json", "{\"error\": \"session not found\"}");
        return;
    }

    write_response(request, 200, "application/json", get_docs(session_id, max_count));
}

// Handler for /judge
void judge_view(const FCGX_Request & request, const vector<pair<string, string>> &params){
    string session_id, doc_id;
    int rel = -2;

    for(auto kv: params){
        if(kv.first == "session_id"){
            session_id = kv.second;
        }else if(kv.first == "doc_id"){
            doc_id = kv.second;
        }else if(kv.first == "rel"){
            rel = stoi(kv.second);
        }
    }

    if(session_id.size() == 0 || doc_id.size() == 0){
        write_response(request, 400, "application/json", "{\"error\": \"Non empty session_id and doc_id required\"}");
    }

    if(SESSIONS.find(session_id) == SESSIONS.end()){
        write_response(request, 404, "application/json", "{\"error\": \"session not found\"}");
        return;
    }

    if(scorer->doc_ids_inv_map.find(doc_id) == scorer->doc_ids_inv_map.end()){
        write_response(request, 404, "application/json", "{\"error\": \"doc_id not found\"}");
        return;
    }

    if(rel < -1 || rel > 1){
        write_response(request, 400, "application/json", "{\"error\": \"rel can either be -1, 0 or 1\"}");
        return;
    }

    BMI *bmi = SESSIONS[session_id];
    bmi->record_judgment(doc_id, rel);

    write_response(request, 200, "application/json", get_docs(session_id, 5));
}

void log_request(const FCGX_Request & request, const vector<pair<string, string>> &params){
    cerr<<string(FCGX_GetParam("RELATIVE_URI", request.envp))<<endl;
    cerr<<FCGX_GetParam("REQUEST_METHOD", request.envp)<<endl;
    for(auto kv: params){
        cerr<<kv.first<<" "<<kv.second<<endl;
    }
    cerr<<endl;
}

void process_request(const FCGX_Request & request) {
    string action = parse_action_from_uri(FCGX_GetParam("RELATIVE_URI", request.envp));
    string method = FCGX_GetParam("REQUEST_METHOD", request.envp);

    vector<pair<string, string>> params;

    if(method == "GET")
        params = parse_query_string(FCGX_GetParam("QUERY_STRING", request.envp));
    else if(method == "POST")
        params = parse_query_string(read_content(request));

    log_request(request, params);

    if(action == "begin"){
        if(method == "POST"){
            begin_session_view(request, params);
        }
    }else if(action == "get_docs"){
        if(method == "GET"){
            get_docs_view(request, params);
        }
    }else if(action == "judge"){
        if(method == "POST"){
            judge_view(request, params);
        }
    }
}

int main(int argc, char **argv){
    AddFlag("--doc-features", "Path of the file with list of document features", string(""));
    AddFlag("--df", "Path of the file with document frequency of each term", string(""));
    AddFlag("--judgments-per-iteration", "Number of docs to judge per iteration (-1 for BMI default)", int(-1));
    AddFlag("--num-iterations", "Set max number of training iterations", int(-1));
    AddFlag("--max-effort", "Set max effort", int(-1));
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

    // Load docs
    auto start = std::chrono::steady_clock::now();
    cerr<<"Loading document features on memory"<<endl;
    scorer = new Scorer(CMD_LINE_STRINGS["--doc-features"]);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
        (std::chrono::steady_clock::now() - start);
    cerr<<"Read "<<scorer->doc_features.size()<<" docs in "<<duration.count()<<"ms"<<endl;

    // Load queries
    features::init(CMD_LINE_STRINGS["--df"]);

    FCGX_Request request;
    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0) {
        process_request(request);
    }


    // Cleanup
    for(auto it: SESSIONS)
        delete it.second;

    delete scorer;

    return 0;
}
