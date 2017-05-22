trecCoreWeb
===========

backend and frontend work

Settings
--------

Moved to settings_.

.. _settings: http://cookiecutter-django.readthedocs.io/en/latest/settings.html

Basic Commands
--------------




Setting Up The Server
^^^^^^^^^^^^^^^^^^^^^

* Install postgres and run it, make sure you create a db called :code:`treccoreweb`
* Create a python3 virtual env, and pip install everything in requirements/base.txt
* Your local db is initially empty, and you will need to create the tables according to the models. To do this, run :bash:`python manage.py makemigrations` and then :bash:`python manage.py migrate`
* For running a uwsgi instance, :bash:`uwsgi --socket 127.0.0.1:8001 --module config.wsgi --master --process 2 --threads 4`


Setting Up Your Users
^^^^^^^^^^^^^^^^^^^^^

* To create a **normal user account**, just go to Sign Up and fill out the form. Once you submit it, you'll see a "Verify Your E-mail Address" page. Go to your console to see a simulated email verification message. Copy the link into your browser. Now the user's email should be verified and ready to go.

* To create an **superuser account**, use this command::

    $ python manage.py createsuperuser

For convenience, you can keep your normal user logged in on Chrome and your superuser logged in on Firefox (or similar), so that you can see how the site behaves for both kinds of users.

Test coverage
^^^^^^^^^^^^^

To run the tests, check your test coverage, and generate an HTML coverage report::

    $ coverage run manage.py test
    $ coverage html
    $ open htmlcov/index.html

Running tests with py.test
~~~~~~~~~~~~~~~~~~~~~~~~~~

::

  $ py.test



Deployment
----------

The following details how to deploy this application.



