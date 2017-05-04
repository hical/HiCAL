# C++ Implementation of [Auto-TAR BMI](http://plg.uwaterloo.ca/~gvcormac/total-recall/)

### Build

```bash
$ g++ -O2 -pthread bmi_cli.cc scorer.cc sofiaml/sf-sparse-vector.cc sofiaml/sf-data-set.cc sofiaml/sf-weight-vector.cc sofiaml/sofia-ml-methods.cc -o bmi_cli
```

### Usage

```
$ ./bmi_cli --help
Command line flag options: 
      --async-mode                  Enable greedy async mode for classifier and rescorer, overrides --judgments-per-iteration
      --doc-features                Path of the file with list of document features
      --help                        Show Help
      --judgment-logpath            Path to log judgments
      --judgments-per-iteration     Number of docs to judge per iteration (-1 for BMI default)
      --max-effort                  Set max effort
      --num-iterations              Set max number of training iterations
      --qrel                        Use the qrel file for judgment
      --query-features              Path of the file with query features
      --threads                     Number of threads to use for scoring
      --topic-id                    Topic id for parsing qrel
```
