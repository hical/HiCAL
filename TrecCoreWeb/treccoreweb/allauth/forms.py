from django import forms


class SignupForm(forms.Form):
    model = None
    labels = {
            'username': ('Username', "Only letters, numbers, '-', '.', and '_'"),
    }
    field_order = ['username', 'email']

    def signup(self, request, user):
        user.save()
