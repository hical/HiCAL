#!/bin/bash
g++ -O2 -pthread src/bmi_cli.cc src/scorer.cc src/sofiaml/sf-sparse-vector.cc src/sofiaml/sf-data-set.cc src/sofiaml/sf-weight-vector.cc src/sofiaml/sofia-ml-methods.cc -o bmi_cli
g++ -O2 -pthread -lfcgi -lfcgi++ src/bmi_fcgi.cc src/scorer.cc src/sofiaml/sf-sparse-vector.cc src/sofiaml/sf-data-set.cc src/sofiaml/sf-weight-vector.cc src/sofiaml/sofia-ml-methods.cc -o bmi_fcgi
