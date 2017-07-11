from config.latinSquare.treatments import all_treatments

from django import forms
from django.contrib.postgres.forms import JSONField

from treccoreweb.progress.models import PostTask
from treccoreweb.progress.models import PreTask
from treccoreweb.progress.models import Task
from treccoreweb.progress.models import TaskSetting
from treccoreweb.topic.models import Topic


class SignupForm(forms.Form):
    model = None
    labels = {
            'username': ('Username', "Only letters, numbers, '-', '.', and '_'"),
    }
    field_order = ['username', 'email', 'sequence', 'treatment_num']
    sequence = JSONField(required=False,
                         initial=[], help_text="If sequence is defined, the treatment "
                                               "sequence and settings will be ignored. ")
    treatment = forms.ChoiceField(choices=((x, x) for x in range(0, 50)))

    def signup(self, request, user):
        user.treatment = int(self.cleaned_data['treatment'])
        treatments = all_treatments[user.treatment]['treatments']
        print(treatments)
        user.sequence = self.cleaned_data['sequence']
        if not self.cleaned_data['sequence']:
            sequence = []
            for t in treatments:
                print(t)
                sequence.append(int(t['topic_num']))
            user.sequence = sequence
        else:
            # default settings for when a sequence is defined
            settings = TaskSetting.objects.get(show_search=True,
                                               toggle_doc=True,
                                               only_show_doc=False)

        is_first = True
        for idx, topic_num in enumerate(user.sequence):
            # Assuming all topics are there and sequence is valid.
            topic = Topic.objects.get(number=topic_num)
            pretask = PreTask.objects.create(username=user)
            posttask = PostTask.objects.create(username=user)

            if not self.cleaned_data['sequence']:
                show_search = treatments[idx]['setting']['show_search']
                toggle_doc = treatments[idx]['setting']['toggle_doc']
                only_show_doc = treatments[idx]['setting']['only_show_doc']

                settings = TaskSetting.objects.get(show_search=show_search,
                                                   toggle_doc=toggle_doc,
                                                   only_show_doc=only_show_doc)

            task = Task.objects.create(username=user,
                                       topic=topic,
                                       pretask=pretask,
                                       posttask=posttask,
                                       setting=settings)
            pretask.task = task
            pretask.save()
            posttask.task = task
            posttask.save()

            if is_first:
                user.current_task = task
                is_first = False

        user.save()
