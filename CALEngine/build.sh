#!/bin/bash
if [[ $# == 0 ]]; then
    echo "Usage: ./build.sh [cli|fcgi]"
    exit 1
fi

if [[ $1 == "cli" ]]; then
    g++ -O2 -pthread src/bmi_cli.cc src/scorer.cc src/sofiaml/sf-sparse-vector.cc src/sofiaml/sf-data-set.cc src/sofiaml/sf-weight-vector.cc src/sofiaml/sofia-ml-methods.cc -o bmi_cli
elif [[ $1 == "fcgi" ]]; then
    g++ -O2 -pthread -lfcgi -lfcgi++ src/bmi_fcgi.cc src/scorer.cc src/sofiaml/sf-sparse-vector.cc src/sofiaml/sf-data-set.cc src/sofiaml/sf-weight-vector.cc src/sofiaml/sofia-ml-methods.cc -o bmi_fcgi
else
    echo "Usage: ./build.sh [cli|fcgi]"
    exit 1
fi
