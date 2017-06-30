from crispy_forms.layout import Submit
from crispy_forms.helper import FormHelper

from django import forms
from treccoreweb.progress.models import Demographic, PreTask, PostTask, LIKERT_SCALE_CHOICES


class DemographicForm(forms.ModelForm):
    """
    Form for demographic questionnaire

    """
    class Meta:
        model = Demographic
        exclude = ["username"]
        other_msg = 'If you answered other in the previous question, please specify:'
        labels = {
            'age': 'Your age',
            'student_degree': 'You are:',
            'student_degree_other': other_msg,
            'student_major': 'Your major',
            'student_major_other': other_msg,
            'language': 'How fluent are you in English',
            'search_engine_usage': 'How often do you search the internet for information '
                                   'using a search engine such as Google, Yahoo Search, '
                                   'or Microsoft Bing:',
            'expertise': 'I am an expert at finding information using search engines '
                         'like Google, Yahoo, and Microsoft Bing:',
            'trouble': 'I often have trouble finding what I am looking '
                       'for on the internet:',
            'help': 'Friends and family turn to me to help them search the internet for '
                    'answers to their questions:',
            'enjoyment': 'I enjoy using search engines like '
                         'Google, Yahoo, and Microsoft Bing:',
            'training': 'Have you ever had special training or education in searching '
                        'or information retrieval?',
            'training_feedback': 'If you answered yes to the previous questions, '
                                 'please describe the training or education:',
            'feedback': 'If you have any general feedback about about you that you think '
                        'is necessary or useful for us to know, please describe it below:'
        }

    submit_name = 'submit-demographic-form'

    def __init__(self, *args, **kwargs):
        super(DemographicForm, self).__init__(*args, **kwargs)
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
