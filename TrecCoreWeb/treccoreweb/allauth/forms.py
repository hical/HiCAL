from django import forms
from django.contrib.postgres.forms import JSONField
from treccoreweb.topic.models import Topic
from treccoreweb.progress.models import Task, PreTask, PostTask
from treccoreweb.users.models import User


class SignupForm(forms.Form):
    model = None
    labels = {
            'username': ('Username', "Only letters, numbers, '-', '.', and '_'"),
    }
    field_order = ['username', 'email', 'sequence']
    sequence = JSONField(initial=[])

    def signup(self, request, user):
        user.sequence = self.cleaned_data['sequence']
        user.save()
        is_first = True
        for topic_num in user.sequence:
            # Assuming all topics are there and sequence is valid.
            topic = Topic.objects.get(number=topic_num)
            pretask = PreTask.objects.create(username=user,
                                             topic=topic)
            posttask = PostTask.objects.create(username=user,
                                               topic=topic)
            task = Task.objects.create(username=user,
                                       topic=topic,
                                       pretask=pretask,
                                       posttask=posttask)
            if is_first:
                user.current_task = task
                user.save()
                is_first = False
