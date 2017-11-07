#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset
set -o xtrace


python manage.py migrate
python manage.py loaddata treccoreweb/progress/fixtures/init_tasksetting.json
python manage.py loaddata treccoreweb/topic/fixtures/init_topics.json
python manage.py collectstatic --no-input
uwsgi --socket 127.0.0.1:8001 --module config.wsgi --master --process 2 --threads 4



# python manage.py runserver 0.0.0.0:8000
