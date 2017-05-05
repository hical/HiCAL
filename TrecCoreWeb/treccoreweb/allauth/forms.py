from django import forms

from treccoreweb.topic.models import Topic


class SignupForm(forms.Form):
    model = None
    labels = {
            'username': ('Username', "Only letters, numbers, '-', '.', and '_'"),
    }

    def signup(self, request, user):
        topic = Topic.objects.create(user=user, title="Test topic")
        user.current_topic = topic
        user.save()
