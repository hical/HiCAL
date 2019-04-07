from crispy_forms.helper import FormHelper
from crispy_forms.layout import Submit
from django import forms
from django.db.models import Q

from hicalweb.progress.models import Task
from hicalweb.topic.models import Topic


class TaskForm(forms.ModelForm):
    """
    Form for creating Task with pre-defined topic

    """
    submit_name = 'submit-task-form'

    class Meta:
        model = Task
        exclude = ["username", "setting", "timespent", "last_activity"]
        help_texts = {
            'max_number_of_judgments': 'Max number of judgments (effort) for this task. '
                                       'Enter 0 or negative number to '
                                       'disable (i.e. no max).',
            'strategy': 'Choose the strategy of retrieval.',
            'show_full_document_content': 'For paragraph strategies, '
                             'indicate whether you would like ability to view full '
                             'document content.',
        }

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

    max_number_of_judgments = forms.IntegerField(required=True)
    strategy = forms.ChoiceField(choices=Task.STRATEGY_CHOICES,
                                 required=True,
                                 help_text=TaskForm.Meta.help_texts.get('strategy'))
    show_full_document_content = forms.BooleanField(required=False,help_text=TaskForm.Meta.help_texts.get('show_full_document_content'))

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
