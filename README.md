# A System for Efficient High-Recall Retrieval

This repo contains the implementation of High-Recall Information Retrieval system, described in the following papers:

+ Nimesh Ghelani, Gordon Cormack, and Mark Smucker. [Refresh Strategies in Continuous Active Learning.] *SIGIR 2018 workshop on Professional Search* 

+ Haotian Zhang, Mustafa Abualsaud, Nimesh Ghelani, Mark Smucker, Gordon Cormack and Maura Grossman. [Effective User Interaction for High-Recall Retrieval: Less is More.] *Submitting*

+ Mustafa Abualsaud, Nimesh Ghelani, Haotian Zhang, Mark Smucker, Gordon Cormack and Maura Grossman. [A System for Efficient High-Recall Retrieval.] *Proceedings of the 41st International ACM SIGIR Conference on Research and Development in Information Retrieval (SIGIR 2018)*

+ Haotian Zhang, Mustafa Abualsaud, Nimesh Ghelani, Angshuman Ghosh, Mark Smucker, Gordon Cormack and Maura Grossman. [UWaterlooMDS at the TREC 2017 Common Core Track.](https://trec.nist.gov/pubs/trec26/papers/UWaterlooMDS-CC.pdf) *(TREC 2017)*

+ Haotian Zhang, Gordon Cormack, Maura Grossman and Mark Smucker. [Evaluating Sentence-Level Relevance Feedback for High-Recall Information Retrieval.](https://arxiv.org/abs/1803.08988)


Our model was evaluated on the standard TREC dataset: TREC Core 2017 Track. 

For component specific details, check the README in their respective directory.

Requirements
-----------

* docker
	- Refer to this installation guideline: https://www.digitalocean.com/community/tutorials/how-to-install-and-use-docker-on-ubuntu-16-04
* docker-compose

Usage
-----------

```bash
# Checkout the repo
$ git clone https://github.com/HTAustin/HiCAL.git
$ cd HiCAL

# build and run docker containers
$ docker-compose -f CoreTrec.yml build
$ docker-compose -f CoreTrec.yml up -d
$ docker-compose -f CoreTrec.yml start

```

Configuration
-----------

Most of the configuration can be performed through these two files

#### config/nginx/nginx.conf

This files controls the nginx server. By default, the CAL is accessed through port 9001
and the web interface is accessed through port 9000. These ports are exposed to 
the outside world by the docker (specified in `CoreTrec.yml`). In this repo, we
use the same nginx instance to serve document and paragraphs to the web interface.

#### CoreTrec.yml

- CAL: `./data` is mounted to the volume `/data` which is meant to be used the `bmi_fcgi`. Keep document features
and related files over there. Modify `command` field if required.
- nginx: the container uses `config/nginx/nginx.conf` as the config. Make changes to the volumes as required.

License
--------
[![GNU GPL v3.0](http://www.gnu.org/graphics/gplv3-127x51.png)](http://www.gnu.org/licenses/gpl.html) 
