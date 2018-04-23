Users
=====

Each document judgments is associated with a user profile.

* To create a **normal user account**, just go to Sign Up and fill out the form.

* To create an **superuser account**, use this command::

    $ python manage.py createsuperuser

For convenience, you can keep your normal user logged in on Chrome and your superuser logged in on Firefox (or similar), so that you can see how the site behaves for both kinds of users.
A superuser can access the platform's admin page by going to ``/admin`` or whatever is set in the settings_.


Configuring Signup Form
^^^^^^^^^^^^^^^^^^^^^^^
:class:`users.models.User` contains the custom user model that you can modify to your need.
:class:`allauth.forms.SignupForm` is a custom signup form that you can change to include extra information that
will be saved in the user model.

Registration and account management is done using django-allauth_.



.. django-allauth_:https://django-allauth.readthedocs.io/en/latest/

