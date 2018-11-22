#ifndef BMI_DOC_SCAL_H
#define BMI_DOC_SCAL_H

#include <unordered_set>
#include "bmi.h"
#include <string>
using namespace std; 

class BMI_doc_scal:public BMI{
    int B = 1;
    int T, N;
    int R;
    vector<vector<int>> stratums;
    public:
    BMI_doc_scal(Seed seed,
        Dataset *documents,
        int num_threads,
        int judgments_per_iteration,
        bool async_mode,
        int training_iterations,
        int _N);

    virtual void record_judgment_batch(std::vector<std::pair<std::string, int>> judgments);

    string strata_to_json(const vector<int> strata){
        string ret = "";
        for(int doc_id: strata){
            if(ret.size() != 0)
                ret += ",";
            ret += "\"" + to_string(doc_id) + "\"";
        }
        return "[" + ret + "]";
    }

    virtual string get_log() {
        string ret = "";
        for(auto &strata: stratums){
            if(ret.size() != 0){
                ret += ",";
            }
            ret += strata_to_json(strata);
        }
        ret = "[" + ret + "]";
        ret = "{ \"stratums\": " + ret + "}";
        return ret;
    }
};

#endif // BMI_DOC_SCAL_H
