# C++ Implementation of [Auto-TAR BMI](http://plg.uwaterloo.ca/~gvcormac/total-recall/)

### Requirements

* libfcgi
* libarchive
* g++
* make
* spawn-fcgi (to run `bmi_fcgi`)

### Command Line Tool

```
$ make bmi_cli
$ ./bmi_cli --help
Command line flag options: 
      --help                Show Help
      --async-mode          Enable greedy async mode for classifier and rescorer,
                            overrides --judgment-per-iteration and --num-iterations

      --df                  Path of the file with list of terms and their document
                            frequencies. The file contains space-separated word and
                            df on every line. Specify only when df information is
                            not encoded in the document features file.

      --jobs                Number of concurrent jobs (topics)
      --threads             Number of threads to use for scoring
      --doc-features        Path of the file with list of document features
      --para-features       Path of the file with list of paragraph features (BMI_PARA)
      --qrel                Qrel file to use for judgment
      --max-effort          Set max effort (number of judgments)
      --max-effort-factor   Set max effort as a factor of recall
      --num-iterations      Set max number of refresh iterations
      --training-iterations  Set number of training iterations
      --judgments-per-iteration  Number of docs to judge per iteration (-1 for BMI default)

      --query               Path of the file with queries (one seed per line; each
                            line is <topic_id> <rel> <string>; can have multiple seeds per topic)

      --judgment-logpath    Path to log judgments. Specify a directory within which
                            topic-specific logs will be generated.

      --mode                Set strategy: (default) BMI_DOC, BMI_PARA, BMI_PARTIAL_RANKING,
                            BMI_ONLINE_LEARNING, BMI_PRECISION_DELAY, BMI_RECENCY_WEIGHTING, BMI_FORGET

      --forget-refresh-period  Period for full training (BMI_FORGET)
      --forget-remember-count  Number of documents to remember (BMI_FORGET)
      --online-learning-delta  Set delta for online learning (BMI_ONLINE_LEARNING)
      --online-learning-refresh-period  Set refresh period for online learning (BMI_ONLINE_LEARNING)
      --partial-ranking-refresh-period  Set refresh period for partial ranking (BMI_PARTIAL_RANKING)
      --partial-ranking-subset-size  Set subset size for partial ranking (BMI_PARTIAL_RANKING)
      --precision-delay-threshold  Set threshold for precision delay (BMI_PRECISION_DELAY)
      --precision-delay-window  Set window size for precision delay (BMI_PRECISION_DELAY)
      --recency-weighting-param  Set parameter for recency weighting (BMI_RECENCY_WEIGHTING)
```

- The document frequency data is encoded within the document features bin file.
`--df` shouldn't be used unless you are using the old document feature format.

### Corpus Parser

This tool generates document features of a given corpus.

The tool takes as input an archived corpus `tar.gz`. Each file in the corpus is treated
as a separate document with the name of the file (excluding the directories) as its ID.
The files are processed as plaintext document. This gives users freedom to clean and
form corpuses in different ways. In order to use the `BMI_PARA` mode in the main tool,
the user needs to split each document such that each new document is a single paragraph.
The name of these paragraph files should be `<doc-id>.<para-id>` (this is used by the tool
to get the parent document for a given paragraph). For this reason, avoid using the character `.` in
the `<doc-id>`.

`--type` as `svmlight` is not used by the main tool and should be used for debugging purposes.
`--out-df` only works when `--type` is `svmlight`.

The bin output format begins with the document frequency data followed by the document feature
data. The document frequency data begins with a `uint32_t` specifying the number of `dfreq` records which follow.
Each `dfreq` record begins with a null terminated string (word) followed by a `uint32_t` document frequency.
The document feature data begins with a `uint32_t` specifying the number of `dfeat` records which follow.
Each `dfeat` record begins with a null terminated string (document ID) followed by a `feature_list`. `feature_list`
begins with a `uint32_t` specifying the number of `feature_pair` which follow. Each `feature_pair`
is a `uint32_t` feature ID followed by a `float` feature weight.


```
$ make corpus_parser
$ ./corpus_parser --help
Command line flag options: 
      --help                Show Help
      --in                  Input corpus archive
      --out                 Output feature file
      --out-df              Output document frequency file
      --type                Output file format:  bin (default) or svmlight
```

### FastCGI based web server
```
$ make bmi_fcgi
$ ./bmi_fcgi --help
Command line flag options: 
      --df                  Path of the file with list of terms and their document frequencies
      --doc-features        Path of the file with list of document features
      --help                Show Help
      --para-features       Path of the file with list of paragraph features
      --threads             Number of threads to use for scoring
```

`fcgi` libraries needs to be present in the system. `bmi_fcgi` uses `libfcgi` to communicate
with any FastCGI capable web server like nginx. `bmi_fcgi` only supports modes `BMI` and `BMI_PARA`.
Install `spawn-fcgi` in your system and run 

```
$ spawn-fcgi -p 8002 -n -- bmi_fcgi --doc-features /path/to/doc/features --df /path/to/df
```

You can interact with the bmi_fcgi server through the HTTP API or the python bindings in `api.py`.

### HTTP API Spec

#### Begin a session

```
POST /begin
Data Params:
    session_id=[string]
    seed_query=[string]
    async=[bool]
    mode=[para,doc, para_scal, doc_scal]
    seed_judgments=doc1:rel1,doc2:rel2
    judgments_per_iteration=[-1 or positive integer]
    
Success Response:
    Code: 200
    Content: {'session-id': [string]}

Error Response:
    Code: 400
    Content: {'error': 'session already exists'}
```

The server can run multiple sessions each of which are identified by a unique `sesion_id`.
The `seed_query` is the initial relevant string used for training. `async` determines if
the server should wait to respond while refreshing. If `async` is set to false, the server
immediately returns the next document to judge from the old ranklist while the refresh is
ongoing in the background. `mode` sets whether to judge on paragraphs or documents.
`seed_documents` can be used to initialize the session with some pre-determined judgments.
It can also be used to restores states of the sessions when restarting the bmi server.
Use positive integer for relevant and negative integer (or zero) for non-relevant.

#### Get document to judge

```
GET /get_docs
URL Params:
    session_id=[string]
    max_count=[int]

Success Response:
    Code: 200
    Content: {'session-id': [string], 'docs': ["doc-1001", "doc-1002", "doc-1010"]}

Error Response:
    Code: 404
    Content: {'error': 'session not found'}
```

#### Submit Judgment

```
POST /judge
Data Params:
    session_id=[string]
    doc_id=[string]
    rel=[-1,1]

Success Response:
    Code: 200
    Content: {'session-id': 'xyz', 'docs': ["doc-1001", "doc-1002", "doc-1010"]}

Error Response:
    Code: 404
    Content: {'error': 'session not found'}
    
    Code: 404
    Content: {'error': 'doc not found'}
    
    Code: 400
    Content: {'error': 'invalid judgment'}
```

The `docs` field in the success response provides next documents to be judged.
This is to save an additional call to `/get_docs`. If `async` is set to false and
the judgement triggers a refresh, the server will finish the refresh before responding.

#### Get Full Ranklist

```
GET /get_ranklist
URL Params:
    session_id=[string]

Success Response:
    Code: 200
    Content (text/plain): newline separated document ids with their scores ordered
                          from highest to lowest

Error Response:
    Code: 404
    Content: {'error': 'session not found'}
```

Returns full ranklist based on the classifier from the last refresh.

#### Delete a session

```
DELETE /delete_session
Data Params:
    session_id: [string]

Success Response:
    Code: 200
    Content: {'session-id': [string]}

Error Response:
    Code: 404
    Content: {'error': 'session not found'}
```

#### Get logs from a session

```
GET /log
Data Params:
    session_id: [string]

Success Response:
    Code: 200
    Content: {...}

Error Response:
    Code: 404
    Content: {'error': 'session not found'}
```

#### Check if docid exits

```
GET /docid_exists
Data Params:
    session_id: [string]
    docid: [string]

Success Response:
    Code: 200
    Content: {'exists': true/false}

Error Response:
    Code: 404
    Content: {'error': 'session not found'}
```
