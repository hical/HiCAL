from crispy_forms.bootstrap import StrictButton
from crispy_forms.helper import FormHelper
from crispy_forms.layout import Submit, Layout, Column, Row, Field, Div
from django import forms
from django.db.models import Q
from hicalweb.progress.models import Session
from hicalweb.topic.models import Topic


class SessionPredefinedTopicForm(forms.ModelForm):
    """
    Form for creating Task with pre-defined topic

    """
    submit_name = 'submit-session-predefine-topic-form'
    prefix = "predefined"

    class Meta:
        model = Session
        exclude = ["username", "setting", "timespent", "last_activity"]
        help_texts = {
            'max_number_of_judgments': '(Optional) Set max number of judgments.',
            'strategy': 'CAL strategy of retrieval.',
        }

    max_number_of_judgments = forms.IntegerField(required=False,
                                                 label="Effort",
                                                 help_text=Meta.help_texts.get(
                                                     'max_number_of_judgments'))

    def __init__(self, *args, **kwargs):
        super(SessionPredefinedTopicForm, self).__init__(*args, **kwargs)
        self.fields['topic'].queryset = Topic.objects.filter(~Q(number=None)).order_by('number')
        self.helper = FormHelper(self)

        self.helper.layout = Layout(
            'topic',
            Row(
                Column('max_number_of_judgments', css_class='form-group col-md-6 mb-0'),
                Column('strategy', css_class='form-group col-md-6 mb-0'),
                css_class='form-row'
            ),
            Div(
                Field('show_full_document_content'),
                css_class='d-none',
                css_id="predefined-show_full_document_content"
            ),
            StrictButton(u'Create session',
                         name=self.submit_name,
                         type="submit",
                         css_class='btn btn-outline-secondary')
        )

    def clean_max_number_of_judgments(self):
        data = self.cleaned_data['max_number_of_judgments']
        if not data:
            data = 0
        return data


class SessionForm(forms.ModelForm):
    """
    Form for creating Session

    """
    submit_name = 'submit-session-form'
    prefix = "topic"

    class Meta:
        model = Topic
        exclude = ["number", "display_description", "narrative"]

    max_number_of_judgments = forms.IntegerField(required=False,
                                                 label="Effort",
                                                 help_text=SessionPredefinedTopicForm.Meta.help_texts.get('max_number_of_judgments'))
    strategy = forms.ChoiceField(choices=Session.STRATEGY_CHOICES,
                                 required=True,
                                 help_text=SessionPredefinedTopicForm.Meta.help_texts.get('strategy'))
    show_full_document_content = forms.BooleanField(required=False)

    def __init__(self, *args, **kwargs):
        super(SessionForm, self).__init__(*args, **kwargs)
        self.fields['description'].widget.attrs['rows'] = 2
        self.helper = FormHelper(self)
        self.helper.layout = Layout(
            Row(
                Column('title', css_class='form-group col-md-4 mb-0'),
                Column('seed_query', css_class='form-group col-md-8 mb-0'),
                css_class='form-row'
            ),
            'description',
            Row(
                Column('max_number_of_judgments', css_class='form-group col-md-6 mb-0'),
                Column('strategy', css_class='form-group col-md-6 mb-0'),
                css_class='form-row'
            ),
            Div(
                Field('show_full_document_content'),
                css_class='d-none',
                css_id="topic-show_full_document_content"
            ),
            StrictButton(u'Create session',
                         name=self.submit_name,
                         type="submit",
                         css_class='btn btn-outline-secondary')
            # Alternative to StrictButton
            # Submit(self.submit_name, u'Create topic and start judging',
            #       css_class='btn')
        )

    def clean_max_number_of_judgments(self):
        data = self.cleaned_data['max_number_of_judgments']
        if not data:
            data = 0
        return data
