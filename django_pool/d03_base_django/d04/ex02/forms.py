from django import forms

class InputForm(forms.Form):
    text_input = forms.CharField(label="Texte")
