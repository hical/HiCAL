#include <iostream>
#include <algorithm>
#include <cmath>

#include <archive.h>
#include <archive_entry.h>
#include "utils/helpers.h"
#include "utils/feature_writer.h"
#include "utils/text_utils.h"
#include "features.h"
#include "utils/feature_parser.h"
#include "utils/simple-cmd-line-helper.h"

using namespace std;
string read_content(archive *&a){
    const void *buff;
    size_t size;
    off_t offset;
    string text;
    while(1){
        int r = archive_read_data_block(a, &buff, &size, &offset);
        if(r == ARCHIVE_EOF || r != ARCHIVE_OK) return text;
        text += string((const char *)buff, size);
    }
}

string get_tempfile(){
    char file_template [] = "/tmp/CAL_XXXXXX";
    mkstemp(file_template);
    return file_template;
}
// Optimized for memory
int main(int argc, char **argv){
    AddFlag("--in", "Input corpus archive", string(""));
    AddFlag("--out", "Output feature file", string(""));
    // Todo: merge df info to bin file
    AddFlag("--out-df", "Output document frequency file", string(""));
    AddFlag("--type", "Output file format:  bin (default) or svmlight", string("bin"));
    AddFlag("--help", "Show Help", bool(false));

    ParseFlags(argc, argv);

    if(CMD_LINE_BOOLS["--help"]){
        ShowHelp();
        return 0;
    }


    string in_filename = CMD_LINE_STRINGS["--in"];
    string pass1_filename = get_tempfile();
    string out_filename = CMD_LINE_STRINGS["--out"];
    bool bin_out = (CMD_LINE_STRINGS["--type"] == "bin");

    cerr<<"Opening file "<<in_filename<<endl;
    archive *a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    int r = archive_read_open_filename(a, in_filename.c_str(), 10240);
    if(r){
        fail(archive_error_string(a), r);
    }

    archive_entry *entry;
    unordered_map<string, uint32_t> token_ids;
    vector<uint32_t> df(1);
    vector<double> idf(1);
    size_t num_docs = 0;

    cerr<<"Beginning Pass 1"<<endl;
    BMITokenizer tokenizer = BMITokenizer();
    // Pass 1: get corpus stat and compute term frequencies
    {
        unique_ptr<CAL::utils::FeatureWriter> fw_1;
        if(bin_out)
            fw_1 = make_unique<CAL::utils::BinFeatureWriter>(pass1_filename);
        else
            fw_1 = make_unique<CAL::utils::SVMlightFeatureWriter>(pass1_filename);

        while (true) {
            r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF)
                break;
            if (r != ARCHIVE_OK) {
                fail(archive_error_string(a), 1);
            }
            if (!(archive_entry_filetype(entry) & AE_IFREG))
                continue;

            string doc_name = (archive_entry_pathname(entry));
            string content = read_content(a);
            num_docs++;
            vector<string> tokens = tokenizer.tokenize(content);

            vector<FeatureValuePair> features;
            for (pair<string, int> token: features::get_tf(tokens)) {
                if (token_ids.count(token.first) == 0) {
                    token_ids[token.first] = df.size();
                    df.push_back(0);
                }
                df[token_ids[token.first]] += 1.0;
                features.push_back({token_ids[token.first], (float) token.second});
            }

            sort(features.begin(), features.end(),
                 [](const FeatureValuePair &a, const FeatureValuePair &b) -> bool { return a.id_ < b.id_; });

            fw_1->write(SfSparseVector(doc_name, features));
            cerr<<num_docs<<" documents processed\r";
        }
    }
    cerr<<endl<<"Computing idf"<<endl;

    // Compute idf
    {
        ofstream df_out(CMD_LINE_STRINGS["--out-df"]);
        for(int i = 1; i < df.size(); i++){
            idf.push_back(df[i] < 2?-1:log(num_docs / (float)df[i]));
        }
        for(auto &token_pair: token_ids){
            if(df[token_pair.second] > 1){
                if(df[token_pair.second] > 1){
                    df_out<<df[token_pair.second]<<" "<<token_pair.first<<endl;
                }
            }
        }
    }

    cerr<<"Beginning Pass 2"<<endl;
    // Pass 2
    unique_ptr<CAL::utils::FeatureParser> fp_1;
    unique_ptr<CAL::utils::FeatureWriter> fw_2;
    if(bin_out){
        fp_1 = make_unique<CAL::utils::BinFeatureParser>(pass1_filename);
        fw_2 = make_unique<CAL::utils::BinFeatureWriter>(out_filename);
    }
    else{
        fp_1 = make_unique<CAL::utils::SVMlightFeatureParser>(pass1_filename);
        fw_2 = make_unique<CAL::utils::SVMlightFeatureWriter>(out_filename);
    }

    unique_ptr<SfSparseVector> spv;
    num_docs = 0;
    while((spv = fp_1->next()) != nullptr){
        vector<FeatureValuePair> features;
        double sum = 0;
        for(auto &f: spv->features_){
            if(df[f.id_] > 1){
                features.push_back({f.id_, (float) ((1 + log(f.value_)) * idf[f.id_])});
                sum += features.back().value_ * features.back().value_;
            }
        }

        sum = sqrt(sum);

        for(auto &f: features){
            f.value_ /= sum;
        }
        fw_2->write(SfSparseVector(spv->doc_id, features));
        num_docs++;
        cerr<<num_docs<<" documents processed\r";
    }
    cerr<<endl;

    archive_read_close(a);
    archive_read_free(a);
}
