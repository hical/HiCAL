#include "dataset/dataset-memory.h"

using namespace std;

DatasetMemory::DatasetMemory(unique_ptr<Featurizer> _featurizer,
                             string filename)
    : Dataset(move(_featurizer)) {
  FILE *fp = fopen(filename.c_str(), "rb");
  setvbuf(fp, NULL, _IOFBF, 1 << 25);

  uint32_t dict_end_offset;
  fread(&dict_end_offset, sizeof(uint32_t), 1, fp);
  std::string term;
  uint32_t df;
  int idx = 0;
  while (ftello(fp) != dict_end_offset) {
    char ch;
    int buf_idx = 0;
    do {
      ch = fgetc(fp);
      buffer[buf_idx++] = ch;
    } while (ch);

    fread(&df, sizeof(uint32_t), 1, fp);
    dictionary[term] = {++idx, (int)df};
    term.clear();
  }
  fread(&num_records, sizeof(num_records), 1, fp);
}
