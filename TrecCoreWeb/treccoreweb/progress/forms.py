from crispy_forms.helper import FormHelper
from crispy_forms.layout import Submit
from django import forms
from django.db.models import Q

from treccoreweb.progress.models import Task
from treccoreweb.topic.models import Topic


class TaskForm(forms.ModelForm):
    """
    Form for creating Task with pre-defined topic

    """
    submit_name = 'submit-task-form'

    class Meta:
        model = Task
        exclude = ["username", "setting", "timespent", "last_activity"]

    def __init__(self, *args, **kwargs):
        super(TaskForm, self).__init__(*args, **kwargs)
        self.fields['topic'].queryset = Topic.objects.filter(~Q(number=None)).order_by('number')
        self.helper = FormHelper(self)
        self.helper.layout.append(
            Submit(self.submit_name, u'Change topic and start judging',
                   css_class='btn btn-warning btn-sm')
        )


class TopicForm(forms.ModelForm):
    """
    Form for creating Topic

    """
    submit_name = 'submit-topic-form'

    class Meta:
        model = Topic
        exclude = ["number", "display_description", "narrative"]

    def __init__(self, *args, **kwargs):
        super(TopicForm, self).__init__(*args, **kwargs)
        self.helper = FormHelper(self)
        self.helper.layout.append(
            Submit(self.submit_name, u'Create topic and start judging',
                   css_class='btn btn-warning btn-sm')
        )
