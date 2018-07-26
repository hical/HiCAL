#ifndef BMI_PARA_SCAL_H
#define BMI_PARA_SCAL_H

#include <unordered_set>
#include "bmi_para.h"

class BMI_para_scal:public BMI_para {
    int B = 1;
    int T, N;
    int R;
    public:
    BMI_para_scal(Seed seed,
        Dataset *documents,
        ParagraphDataset *paragraphs,
        int num_threads,
        int training_iterations, int N);

    virtual void record_judgment_batch(std::vector<std::pair<std::string, int>> judgments);
};

#endif // BMI_PARA_SCAL_H
