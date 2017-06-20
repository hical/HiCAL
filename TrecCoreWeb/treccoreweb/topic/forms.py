from crispy_forms.layout import Submit
from crispy_forms.helper import FormHelper

from django import forms
from treccoreweb.topic.models import Topic, PreTask, PostTask, LIKERT_SCALE_CHOICES


class TopicForm(forms.ModelForm):
    """
    Form for creating a new Topic

    """
    submit_name = 'submit-topic-form'

    class Meta:
        model = Topic
        fields = ['title', 'number', 'seed_query', 'description']

    def __init__(self, *args, **kwargs):
        super(TopicForm, self).__init__(*args, **kwargs)
        instance = getattr(self, 'instance', None)
        if instance and instance.pk:
            self.fields['seed_query'].widget.attrs['readonly'] = True
            self.fields['number'].widget.attrs['readonly'] = True

        self.helper = FormHelper(self)
        self.helper.layout.append(
            Submit(self.submit_name, u'Submit',
                   css_class='btn btn-primary')
        )


class PreTaskForm(forms.ModelForm):
    """
    Form for updating Pretask

    """
    submit_name = 'submit-pretask-form'
    familiarity = forms.CharField(
        widget=forms.Select(choices=LIKERT_SCALE_CHOICES),
        label=u'Question on familiarity?')
    difficulty = forms.CharField(
        widget=forms.Select(choices=LIKERT_SCALE_CHOICES),
        label=u'Question on difficulty?')
    feedback = forms.CharField(
        widget=forms.Textarea(attrs={'rows': 5,
                                     'cols': 80}
                              ),
        label=u'Question on feedback?',
        required=False
    )

    class Meta:
        model = PreTask
        fields = ['familiarity', 'difficulty', 'feedback']

    def __init__(self, *args, **kwargs):
        super(PreTaskForm, self).__init__(*args, **kwargs)
        self.helper = FormHelper(self)
        self.helper.layout.append(
            Submit(self.submit_name, u'Submit',
                   css_class='btn btn-primary')
        )


class PostTaskForm(forms.ModelForm):
    """
    Form for updating Posttask

    """
    submit_name = 'submit-posttask-form'
    difficulty = forms.CharField(
        widget=forms.Select(choices=LIKERT_SCALE_CHOICES),
        label=u'Question on difficulty?')
    confidence = forms.CharField(
        widget=forms.Select(choices=LIKERT_SCALE_CHOICES),
        label=u'Question on confidence?')
    mood = forms.CharField(
        widget=forms.Select(choices=LIKERT_SCALE_CHOICES),
        label=u'Question on nood?')
    experience = forms.CharField(
        widget=forms.Select(choices=LIKERT_SCALE_CHOICES),
        label=u'Question on experience?')

    feedback = forms.CharField(
        widget=forms.Textarea(attrs={'rows': 5,
                                     'cols': 80}
                              ),
        label=u'Question on feedback?',
        required=False
    )

    class Meta:
        model = PostTask
        fields = ['difficulty', 'confidence', 'mood', 'experience', 'feedback']

    def __init__(self, *args, **kwargs):
        super(PostTaskForm, self).__init__(*args, **kwargs)
        self.helper = FormHelper(self)
        self.helper.layout.append(
            Submit(self.submit_name, u'Submit',
                   css_class='btn btn-primary')
        )
