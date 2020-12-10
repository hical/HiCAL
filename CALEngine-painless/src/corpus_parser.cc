#include <archive.h>
#include <archive_entry.h>

#include <iostream>

#include "dataset/dataset-memory.h"
#include "featurizer/tfidf.h"
#include "utils/logging.h"
#include "utils/simple-cmd-line-helper.h"

using namespace std;
string read_content(archive *&a) {
    const void *buff;
    size_t size;
    off_t offset;
    string text;
    while (1) {
        int r = archive_read_data_block(a, &buff, &size, &offset);
        if (r == ARCHIVE_EOF || r != ARCHIVE_OK) return text;
        text += string((const char *)buff, size);
    }
}

string get_tempfile() {
    char file_template[] = "/tmp/CAL_XXXXXX";
    mkstemp(file_template);
    return file_template;
}

archive *read_archive(const string &filename) {
    archive *a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    int r = archive_read_open_filename(a, filename.c_str(), 10240);
    if (r) {
        FATAL(archive_error_string(a));
    }
    return a;
}

void fit_featurizer(Featurizer &featurizer, const string &filename) {
    INFO("Fitting featurizer");
    archive *a = read_archive(filename);
    archive_entry *entry;
    size_t num_docs = 0;

    while (true) {
        int r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) break;
        if (r != ARCHIVE_OK) {
            FATAL(archive_error_string(a));
        }
        if (!(archive_entry_filetype(entry) & AE_IFREG)) continue;

        string doc_name = (archive_entry_pathname(entry));
        if (doc_name.find_last_of('/') != doc_name.npos) {
            doc_name = doc_name.substr(doc_name.find_last_of('/') + 1);
        }
        featurizer.fit(read_content(a));
        num_docs++;
        cerr << num_docs << " documents processed\r";
    }
    featurizer.finalize();
    cerr << endl;

    archive_read_close(a);
    archive_read_free(a);
}

void fit_dataset(Dataset &dataset, const string &filename) {
    INFO("Fitting dataset");
    archive *a = read_archive(filename);
    archive_entry *entry;
    size_t num_docs = 0;

    while (true) {
        int r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) break;
        if (r != ARCHIVE_OK) {
            FATAL(archive_error_string(a));
        }
        if (!(archive_entry_filetype(entry) & AE_IFREG)) continue;

        string doc_name = (archive_entry_pathname(entry));
        if (doc_name.find_last_of('/') != doc_name.npos) {
            doc_name = doc_name.substr(doc_name.find_last_of('/') + 1);
        }
        dataset.add(doc_name, read_content(a));
        num_docs++;
        cerr << num_docs << " documents processed\r";
    }
    cerr << endl;

    archive_read_close(a);
    archive_read_free(a);
}

int main(int argc, char **argv) {
    AddFlag("--in", "Input corpus archive", string(""));
    AddFlag("--out", "Output featurized dataset file", string(""));
    AddFlag("--out-feature", "Output featurizer file", string(""));
    AddFlag("--dataset-format",
            "Output file format:  bin (default) or svmlight", string("bin"));
    AddFlag("--help", "Show Help", bool(false));

    ParseFlags(argc, argv);

    if (CMD_LINE_BOOLS["--help"]) {
        ShowHelp();
        return 0;
    }

    string in_filename = CMD_LINE_STRINGS["--in"];
    string out_filename = CMD_LINE_STRINGS["--out"];
    string out_feat_filename = CMD_LINE_STRINGS["--out-feature"];
    string dataset_format = CMD_LINE_STRINGS["--dataset-format"];

    DatasetMemory::DatasetFormat df;
    if (dataset_format == "bin") {
        df = DatasetMemory::BIN;
    } else if (dataset_format == "svmlight") {
        df = DatasetMemory::SVMLIGHT;
    } else {
        FATAL("Invalid dataset format " + string(dataset_format));
        return 1;
    }

    auto tfidf = unique_ptr<TFIDFFeaturizer>(new TFIDFFeaturizer());
    tfidf->set_min_df(2);
    fit_featurizer(*tfidf, in_filename);
    tfidf->write(out_feat_filename);
    DatasetMemory dataset(move(tfidf));
    fit_dataset(dataset, in_filename);
    dataset.write(out_filename, df);
}
