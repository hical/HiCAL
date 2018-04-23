# C++ Implementation of [Auto-TAR BMI](http://plg.uwaterloo.ca/~gvcormac/total-recall/)

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
      --qrel                Use the qrel file for judgment
      --max-effort          Set max effort (number of judgments)
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
    mode=[para,doc]
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
