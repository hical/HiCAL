# Useful docker commands

* Delete all containers: `docker rm $(docker ps -a -q)` 
* Delete all volumns: `docker volume rm $(docker volume ls -q)`
* Build a container with yml:  `docker-compose -f <YML_FILE>.yml build`
* Run migrations: `docker-compose -f <YML_FILE>.yml.yml run django python manage.py migrate`
* Create superuser: `docker-compose -f <YML_FILE>.yml.yml run django python manage.py createsuperuser`
* Run a container default command:  `docker-compose -f <YML_FILE>.yml up`



