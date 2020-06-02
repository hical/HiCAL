from crispy_forms.helper import FormHelper, Layout
from django import forms


class UploadForm(forms.Form):
    """
    Form for uploading judgments for current session

    """
    submit_name = 'upload-form'

    csv_file = forms.FileField(required=True,
                               label="CSV File",
                               widget=forms.FileInput)
    train_model = forms.BooleanField(
        required=False,
        label="Use judgments to train CAL model.",
        widget=forms.CheckboxInput
    )
    update_existing = forms.BooleanField(
        required=False,
        label="Update existing judgments"
    )

    def __init__(self, *args, **kwargs):
        super(UploadForm, self).__init__(*args, **kwargs)
        self.helper = FormHelper(self)
        self.helper.use_custom_control = True
        self.helper.help_text_inline = True
        self.helper.form_tag = False

        self.helper.layout = Layout(
            'csv_file',
            'train_model',
            'update_existing',
        )

