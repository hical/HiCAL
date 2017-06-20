from django import forms

from treccoreweb.topic.models import Topic


class SignupForm(forms.Form):
    model = None
    labels = {
            'username': ('Username', "Only letters, numbers, '-', '.', and '_'"),
    }

    def signup(self, request, user):
        # TODO create the five tasks and their pre/post task instances
        # TODO update current user task
        pass
