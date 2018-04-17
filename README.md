# A System for Efficient High-Recall Retrieval

This repo contains the implementation of High-Recall Information Retrieval system, described in the following papers:

+ Haotian Zhang, Mustafa Abualsaud, Nimesh Ghelani, Mark Smucker, Gordon Cormack and Maura Grossman. [Effective User Interaction for High-Recall Retrieval: Less is More.] *Submitting*.

+ Mustafa Abualsaud, Nimesh Ghelani, Haotian Zhang, Mark Smucker, Gordon Cormack and Maura Grossman. [A System for Efficient High-Recall Retrieval.] *Proceedings of the 41st International ACM SIGIR Conference on Research and Development in Information Retrieval (SIGIR 2018)*

+ Haotian Zhang, Mustafa Abualsaud, Nimesh Ghelani, Angshuman Ghosh, Mark Smucker, Gordon Cormack and Maura Grossman. [UWaterlooMDS at the TREC 2017 Common Core Track.](https://trec.nist.gov/pubs/trec26/papers/UWaterlooMDS-CC.pdf) *(TREC 2017)*.

+ Haotian Zhang, Gordon Cormack, Maura Grossman and Mark Smucker. [Evaluating Sentence-Level Relevance Feedback for High-Recall Information Retrieval.](https://arxiv.org/abs/1803.08988).


Our model was evaluated on the standard TREC dataset: TREC Core 2017 Track. 

Getting Started
-----------

``1.`` Checkout our repo:
```
git clone https://github.com/HTAustin/CAL-P
```

``2.`` Deploy the CAL back-end:
```
$ cd CALEngine/
``` 

``3.`` Deploy the front-end:
```
$ cd TrecCoreWeb/
``` 

Running
--------
``1.`` There are several command line parameters to specify for running our model:
```
-dataset, the dataset you want to evaluate 
```


License
--------
[![GNU GPL v3.0](http://www.gnu.org/graphics/gplv3-127x51.png)](http://www.gnu.org/licenses/gpl.html) 