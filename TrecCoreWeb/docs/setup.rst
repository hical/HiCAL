
Setting Up The Server
=====================


* The platform requires a database for logging and storing document judgments. Install postgres and run it, make sure you create a db called ``<projectname>``, or whatever you have set in settings_. To create a db, use this command::

    $ createdb <projectname>

* Create a python3 virtual env, and pip install everything in ``requirements/base.txt``::

    $ pip install -r requirements/base.txt

* If you are running it locally, you might also want to install everything in ``requirements/local.txt``::


    $ pip install -r requirements/local.txt

* Your local db is initially empty, and you will need to create the tables according to the models. To do this, run ``python manage.py makemigrations`` and then ``python manage.py migrate``.
* For running a uwsgi instance, ``uwsgi --socket 127.0.0.1:8001 --module config.wsgi --master --process 2 --threads 4``
* To run locally, run ``python manage.py runserver``


Component Settings
^^^^^^^^^^^^^^^^^^
Communication from and to the component are done through

