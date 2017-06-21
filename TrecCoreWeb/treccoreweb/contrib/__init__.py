"""
To understand why this file is here, please read:

http://cookiecutter-django.readthedocs.io/en/latest/faq.html#why-is-there-a-django-contrib-sites-directory-in-cookiecutter-django
"""

from django.contrib import admin
from django.contrib.auth.models import Group
from django.contrib.sites.models import Site

admin.site.unregister(Group)
admin.site.unregister(Site)
