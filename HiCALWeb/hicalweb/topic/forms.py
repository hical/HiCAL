from crispy_forms.layout import Submit
from crispy_forms.helper import FormHelper

from django import forms
from hicalweb.topic.models import Topic


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

