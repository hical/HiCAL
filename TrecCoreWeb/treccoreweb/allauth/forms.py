from config.latinSquare.treatments import all_treatments

from django import forms
from django.contrib.postgres.forms import JSONField


class SignupForm(forms.Form):
    model = None
    labels = {
            'username': ('Username', "Only letters, numbers, '-', '.', and '_'"),
    }
    field_order = ['username', 'email']

    def signup(self, request, user):
        user.save()
