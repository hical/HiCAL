HiCAL
===========

HiCAL is a high-recall retrieval platform the supports different retrieval methods.
This repository contains the platform server which is responsible for displaying the document assessment interfaces to the user,
configuring the system, and communicating requests and responses to and from each retrieval component.



Settings
--------

For the platform settings, please check out settings_.

.. _settings: http://url.com



Basic Commands
--------------


Setting Up The Server
^^^^^^^^^^^^^^^^^^^^^

* The platform requires a database for logging and storing document judgments. Install postgres and run it, make sure you create a db called ``hicalweb``, or whatever you have set in settings_. To create a db, use this command::

    $ createdb hicalweb

* Create a python3 virtual env, and pip install everything in ``requirements/base.txt``::

    $ pip install -r requirements/base.txt

* If you are running it locally, you might also want to install everything in ``requirements/local.txt``::


    $ pip install -r requirements/local.txt

* Your local db is initially empty, and you will need to create the tables according to the models. To do this, run ``python manage.py makemigrations`` and then ``python manage.py migrate``.
* For running a uwsgi instance, ``uwsgi --socket 127.0.0.1:8001 --module config.wsgi --master --process 2 --threads 4``
* To run locally, run ``python manage.py runserver``


Setting Up Your Users
^^^^^^^^^^^^^^^^^^^^^

* To create a **normal user account**, just go to Sign Up and fill out the form.

* To create an **superuser account**, use this command::

    $ python manage.py createsuperuser

For convenience, you can keep your normal user logged in on Chrome and your superuser logged in on Firefox (or similar), so that you can see how the site behaves for both kinds of users.
A superuser can access the platform's admin page by going to ``/admin`` or whatever is set in the settings_.


License
----------

|gpl|_

.. |gpl| image:: http://www.gnu.org/graphics/gplv3-127x51.png
.. _gpl: http://www.gnu.org/licenses/gpl.html
