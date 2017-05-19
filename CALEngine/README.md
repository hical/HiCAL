# C++ Implementation of [Auto-TAR BMI](http://plg.uwaterloo.ca/~gvcormac/total-recall/)

### Command Line Tool

```
$ make cli
$ ./bmi_cli --help
Command line flag options: 
      --async-mode          Enable greedy async mode for classifier and rescorer, overrides
                            --judgment-per-iteration and --num-iterations
      --df                  Path of the file with document frequency of each term
      --doc-features        Path of the file with list of document features
      --help                Show Help
      --judgment-logpath    Path to log judgments
      --judgments-per-iteration  Number of docs to judge per iteration (-1 for BMI default)
      --max-effort          Set max effort
      --num-iterations      Set max number of training iterations
      --qrel                Use the qrel file for judgment
      --query               Path of the file with queries (odd lines containing topic-id and
                            even lines containingrespective query string)
      --threads             Number of threads to use for scoring
```

### FastCGI based web server
```
$ make fcgi
```

`fcgi` libraries needs to be present in the system. `bmi_fcgi` uses `libfcgi` to communicate
with any FastCGI capable web server like nginx (use the `fcgi_config/nginx.conf` and
`fcgi_config/fastcgi.conf`). Install `spawn-fcgi` in your system and run 

```
$ spawn-fcgi -p 9888 -n -- bmi_fcgi --doc-features /path/to/doc/features --df /path/to/df --async-mode
$ # Optional - restoring sessions
$ cd ../TrecCoreWeb
$ # Switch to virtualenv
$ python restore.py
```

### HTTP API Specs [1](https://gist.github.com/iros/3426278)

Server: `129.97.84.14/CAL`

#### Begin a session
* **URL**

    /begin

* **Method**
   
   `POST`

* **URL Params**
   
    None

* **Data Params**

    `session_id=[string]`, `seed_query=[string]`
    
* **Success Response**

    **Code:** 200 <br />
    **Content:** `{'session-id': 'xyz'}`

* **Error Response**

    **Code:** 400 <br />
    **Content:** `{'error': 'session already exists'}`

#### Get document to judge

* **URL**

    /get_docs

* **Method**
   
   `GET`

* **URL Params**
   
    `session_id=[string]`, `max_count=[int]`

* **Data Params**

    None
    
* **Success Response**

    **Code:** 200 <br />
    **Content:** `{'session-id': 'xyz', 'docs': ["doc-1001", "doc-1002", "doc-1010"]}`

* **Error Response**

    **Code:** 404 <br />
    **Content:** `{'error': 'session not found'}`

#### Submit Judgment

* **URL**

    /judge

* **Method**
   
   `POST`

* **URL Params**
   
    None

* **Data Params**

    `session_id=[string]`, `doc_id=[string]`, `rel=[-1,0,1]`
    
* **Success Response**

    **Code:** 200 <br />
    **Content:** `{'session-id': 'xyz', 'docs': ["doc-1001", "doc-1002", "doc-1010"]}`

* **Error Response**

    **Code:** 404 <br />
    **Content:** `{'error': 'session not found'}`
    
    **Code:** 404 <br />
    **Content:** `{'error': 'doc not found'}`
    
    **Code:** 400 <br />
    **Content:** `{'error': 'invalid judgment'}`
