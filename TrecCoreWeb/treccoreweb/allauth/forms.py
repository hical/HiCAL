from django import forms

from treccoreweb.topic.models import Topic


class SignupForm(forms.Form):
    model = None
    labels = {
            'username': ('Username', "Only letters, numbers, '-', '.', and '_'"),
    }

    def signup(self, request, user):
        topic = Topic.objects.create(username=user, title="Please create a new topic",
                                     seed_query="",
                                     description="This is the default topic initialized "
                                                 "after signing up. Please create a new "
                                                 "topic, activate it, and delete the "
                                                 "this topic.")
        user.current_topic = topic
        user.save()
