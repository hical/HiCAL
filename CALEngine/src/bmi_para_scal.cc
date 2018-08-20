#include <iostream>
#include <algorithm>
#include "bmi_para_scal.h"
using namespace std;

BMI_para_scal::BMI_para_scal(Seed _seed,
        Dataset *_documents,
        ParagraphDataset *_paragraphs,
        int _num_threads,
        int _training_iterations, int _N, vector<pair<string, int>> &seed_judgments)
    :BMI_para(_seed, _documents, _paragraphs, _num_threads, -1, false, _training_iterations)
{
    N = _N;
    T = N;
    R = 0;
    judgments_per_iteration = B;
    for(auto &seed_judgment: seed_judgments){
        add_to_training_cache(documents->get_index(seed_judgment.first), seed_judgment.second);
    }
    perform_iteration();
    stratums.push_back(vector<int>());
    for(int doc_id: judgment_queue)
        stratums.back().push_back(doc_id);
    B = B + ceil(B/10.0);
}

void BMI_para_scal::record_judgment_batch(vector<pair<string, int>> _judgments){
    lock_guard<mutex> lock(judgment_list_mutex);
    for(const auto &judgment: _judgments){
        size_t id = documents->get_index(judgment.first);
        add_to_training_cache(id, judgment.second);
        for(int i = (int)judgment_queue.size() - 1; i >= 0; i--){
            if(paragraphs->translate_index(judgment_queue[i]) == id){
                judgment_queue.erase(judgment_queue.begin() + i);
                if(judgment.second > 0) R++;
                break;
            }
        }
    }

    if(judgment_queue.size() == 0){
        cerr<<"Refreshing"<<endl;
        cerr<<"R = "<<R<<endl;
        if(R >= T) {
            T <<= 1;
            cerr<<"Doubling T to "<<T<<endl;
        }
        cerr<<"Batch Size = "<<B<<endl;
        judgments_per_iteration = B;
        vector<int> batch = perform_training_iteration();
        stratums.push_back(vector<int>());
        for(int doc_id: batch)
            stratums.back().push_back(doc_id);

        int n = ceil(B*N/(float)T);
        cerr<<"Sampling "<<n<<" documents"<<endl;
        vector<int> selector(batch.size());
        for(int i = 0; i < selector.size(); i++)
            selector[i] = (i < n?1:0);
        shuffle(batch.begin(), batch.end(), rand_generator);
        for(int i = 0; i < batch.size(); i++){
            if(selector[i]) judgment_queue.push_back(batch[i]);
            judgments[paragraphs->translate_index(batch[i])] = -2;
        }
        B = B + ceil(B/10.0);
    }
}
