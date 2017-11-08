#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset


celery -A treccoreweb.taskapp worker -l INFO
