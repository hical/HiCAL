#include <iostream>
#include <algorithm>
#include <cmath>

#include <archive.h>
#include <archive_entry.h>
#include "utils/feature_writer.h"
#include "utils/text_utils.h"
#include "utils/utils.h"
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
    AddFlag("--para-in", "Input corpus paragraph archive", string(""));
    AddFlag("--para-out", "Output paragraph feature file", string(""));
    AddFlag("--concatenated", "Input corpus archive is concatenated", bool(false));
    AddFlag("--help", "Show Help", bool(false));

    ParseFlags(argc, argv);

    if(CMD_LINE_BOOLS["--help"]){
        ShowHelp();
        return 0;
    }


    string in_filename = CMD_LINE_STRINGS["--in"];
    string para_in_filename = CMD_LINE_STRINGS["--para-in"];
    string pass1_filename = get_tempfile();
    string out_filename = CMD_LINE_STRINGS["--out"];
    string para_out_filename = CMD_LINE_STRINGS["--para-out"];
    bool bin_out = (CMD_LINE_STRINGS["--type"] == "bin");
    bool is_concatenated = CMD_LINE_BOOLS["--concatenated"];

    cerr<<"Opening file "<<in_filename<<endl;
    archive *a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if(is_concatenated)
        archive_read_set_options(a, "read_concatenated_archives");

    int r = archive_read_open_filename(a, in_filename.c_str(), 10240);
    if(r){
        fail(archive_error_string(a), r);
    }

    archive_entry *entry;
    unordered_map<string, uint32_t> token_ids;
    vector<double> idf(1);
    vector<pair<string, uint32_t>> dictionary;
    size_t num_docs = 0;

    cerr<<"Beginning Pass 1"<<endl;
    BMITokenizer tokenizer = BMITokenizer();
    // Pass 1: get corpus stat and compute term frequencies
    {
        unique_ptr<FeatureWriter> fw_1;
        if(bin_out)
            fw_1 = make_unique<BinFeatureWriter>(pass1_filename, vector<pair<string, uint32_t>>());
        else
            fw_1 = make_unique<SVMlightFeatureWriter>(pass1_filename, "", vector<pair<string, uint32_t>>());

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
            if(doc_name.find_last_of('/') != doc_name.npos){
                doc_name = doc_name.substr(doc_name.find_last_of('/') + 1);
            }
            string content = read_content(a);
            num_docs++;
            vector<string> tokens = tokenizer.tokenize(content);

            vector<FeatureValuePair> features;
            for (pair<string, int> token: features::get_tf(tokens)) {
                if (token_ids.count(token.first) == 0) {
                    dictionary.push_back({token.first, 0});
                    token_ids[token.first] = dictionary.size();
                }
                dictionary[token_ids[token.first]-1].second += 1.0;
                features.push_back({token_ids[token.first], (float) token.second});
            }

            sort(features.begin(), features.end(),
                 [](const FeatureValuePair &a, const FeatureValuePair &b) -> bool { return a.id_ < b.id_; });

            fw_1->write(SfSparseVector(doc_name, features));
            cerr<<num_docs<<" documents processed\r";
            /* if(num_docs == 1000) */
            /*     break; */
        }
        fw_1->finish();
    }
    cerr<<endl<<"Computing idf"<<endl;

    vector<int> new_ids(dictionary.size());
    for(int i = 0; i < dictionary.size(); i++){
        new_ids[i] = i;
    }
    // Compute idf
    {
        int end = dictionary.size() - 1;
        for(int i = 0; i <= end; i++){
            if(dictionary[i].second < 2){
                while(end > i){
                    if(dictionary[end].second > 1){
                        swap(dictionary[i], dictionary[end]);
                        new_ids[i] = end;
                        new_ids[end] = i;
                        break;
                    }
                    end--;
                }
            }
            idf.push_back(dictionary[i].second < 2?-1:log(num_docs / (float)dictionary[i].second));
        }
        while(dictionary[end].second < 2)
            end--;
        dictionary = vector<pair<string, uint32_t>>(dictionary.begin(), dictionary.begin() + end + 1);
    }

    cerr<<"Beginning Pass 2"<<endl;
    // Pass 2
    unique_ptr<FeatureParser> fp_1;
    unique_ptr<FeatureWriter> fw_2;
    if(bin_out){
        fp_1 = make_unique<BinFeatureParser>(pass1_filename);
        fw_2 = make_unique<BinFeatureWriter>(out_filename, dictionary);
    }
    else{
        fp_1 = make_unique<SVMlightFeatureParser>(pass1_filename, "");
        fw_2 = make_unique<SVMlightFeatureWriter>(out_filename, CMD_LINE_STRINGS["--out-df"], dictionary);
    }

    unique_ptr<SfSparseVector> spv;
    num_docs = 0;
    while((spv = fp_1->next()) != nullptr){
        vector<FeatureValuePair> features;
        double sum = 0;
        for(auto &f: spv->features_){
            if(f.id_ != 0){
                f.id_ = new_ids[f.id_-1] + 1;
                if(f.id_ - 1 < dictionary.size() && dictionary[f.id_-1].second > 1){
                    features.push_back({f.id_, (float) ((1 + log(f.value_)) * idf[f.id_])});
                    sum += features.back().value_ * features.back().value_;
                }
            }
        }

        sum = sqrt(sum);

        for(auto &f: features){
            f.value_ /= sum;
        }

        sort(features.begin(), features.end(),
             [](const FeatureValuePair &a, const FeatureValuePair &b) -> bool { return a.id_ < b.id_; });

        fw_2->write(SfSparseVector(spv->doc_id, features));
        num_docs++;
        cerr<<num_docs<<" documents processed\r";
    }
    cerr<<endl;
    fw_2->finish();

    archive_read_close(a);
    archive_read_free(a);

    if(para_in_filename.length() > 0){
        cerr<<"Generating Paragraph features"<<endl;
        archive *a = archive_read_new();
        archive_read_support_format_all(a);
        archive_read_support_filter_all(a);

        unique_ptr<FeatureWriter> para_fw;
        if(bin_out){
            para_fw = make_unique<BinFeatureWriter>(para_out_filename, dictionary);
        }
        else{
            para_fw = make_unique<SVMlightFeatureWriter>(para_out_filename, CMD_LINE_STRINGS["--out-df"], dictionary);
        }

        r = archive_read_open_filename(a, para_in_filename.c_str(), 10240);
        if(r){
            fail(archive_error_string(a), r);
        }

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
            if(doc_name.find_last_of('/') != doc_name.npos){
                doc_name = doc_name.substr(doc_name.find_last_of('/') + 1);
            }
            string content = read_content(a);
            num_docs++;
            vector<string> tokens = tokenizer.tokenize(content);

            vector<FeatureValuePair> features;
            double sum = 0;
            for (pair<string, int> token: features::get_tf(tokens)) {
                if (token_ids.count(token.first) == 0) {
                    continue;
                }
                uint32_t id = new_ids[token_ids[token.first]-1] + 1;
                if(id - 1 < dictionary.size() && dictionary[id-1].second > 1){
                    float wt = (float) (token.second * idf[id]);
                    features.push_back({id, wt});
                    sum += wt * wt;
                }
            }
            sum = max(20.0, sqrt(sum));
            for(auto &f: features){
                f.value_ /= sum;
            }

            sort(features.begin(), features.end(),
                 [](const FeatureValuePair &a, const FeatureValuePair &b) -> bool { return a.id_ < b.id_; });

            para_fw->write(SfSparseVector(doc_name, features));
            cerr<<num_docs<<" paragraphs processed\r";
            /* if(num_docs == 1000) */
            /*     break; */
        }
        para_fw->finish();
    }
}
