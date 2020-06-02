#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset
set -o xtrace


python manage.py makemigrations && python manage.py migrate

# You can import topics by
# importing via django fixtures
python manage.py loaddata hicalweb/topic/fixtures/init_topics.json
# or via django commands, reading topics.xml file in /topic/fixtures/
# (update topics/management/commands/import_topics as necessary)
#python manage.py import_topics

python manage.py collectstatic --no-input
echo "from django.contrib.auth import get_user_model; User = get_user_model(); User.objects.create_superuser('admin', 'admin@myproject.com', 'password')" | python manage.py shell
uwsgi --socket 0.0.0.0:8001 --module config.wsgi --master --process 2 --threads 4


# python manage.py runserver 0.0.0.0:8000
