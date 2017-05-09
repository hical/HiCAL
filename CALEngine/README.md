# C++ Implementation of [Auto-TAR BMI](http://plg.uwaterloo.ca/~gvcormac/total-recall/)

### Build and Run

```bash
$ ./build.sh
$ ./bmi_cli
```

### Usage

```
$ ./bmi_cli --help
Command line flag options: 
      --async-mode          Enable greedy async mode for classifier and rescorer, overrides --judgment-per-iteration and --num-iterations
      --df                  Path of the file with document frequency of each term
      --doc-features        Path of the file with list of document features
      --help                Show Help
      --judgment-logpath    Path to log judgments
      --judgments-per-iteration  Number of docs to judge per iteration (-1 for BMI default)
      --max-effort          Set max effort
      --num-iterations      Set max number of training iterations
      --qrel                Use the qrel file for judgment
      --query               Path of the file with queries (odd lines containing topic-id and even lines containingrespective query string)
      --threads             Number of threads to use for scoring
```
