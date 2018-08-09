#ifndef BMI_PARA_SCAL_H
#define BMI_PARA_SCAL_H

#include <unordered_set>
#include "bmi_para.h"

class BMI_para_scal:public BMI_para {
    int B = 1;
    int T, N;
    int R;
    vector<vector<int>> stratums;
    public:
    BMI_para_scal(Seed seed,
        Dataset *documents,
        ParagraphDataset *paragraphs,
        int num_threads,
        int training_iterations, int N, std::vector<std::pair<std::string, int>> &seed_judgments);

    virtual void record_judgment_batch(std::vector<std::pair<std::string, int>> judgments);

    string strata_to_json(const vector<int> strata){
        string ret = "";
        for(int doc_id: strata){
            if(ret.size() != 0)
                ret += ",";
            ret += "\"" + paragraphs->get_id(doc_id) + "\"";
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

#endif // BMI_PARA_SCAL_H
