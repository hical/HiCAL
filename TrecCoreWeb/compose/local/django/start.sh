#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset
set -o xtrace


python manage.py migrate
python manage.py loaddata treccoreweb/progress/fixtures/init_tasksetting.json
python manage.py loaddata treccoreweb/topic/fixtures/init_topics.json
python manage.py runserver 0.0.0.0:8000
