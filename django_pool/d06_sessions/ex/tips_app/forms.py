from django import forms
from django.contrib.auth import authenticate, login
from django.contrib.auth.models import User
from .models import Tip


class RegisterForm(forms.Form):
    username = forms.CharField(label="Nom d'utilisateur", max_length=150)
    password = forms.CharField(label="Mot de passe", widget=forms.PasswordInput)
    password2 = forms.CharField(label="Vérification du mot de passe", widget=forms.PasswordInput)

    def clean(self):
        cleaned = super().clean()
        username = cleaned.get('username')
        password = cleaned.get('password')
        password2 = cleaned.get('password2')

        if not username:
            self.add_error('username', "Ce champ est requis.")
        if not password:
            self.add_error('password', "Ce champ est requis.")
        if not password2:
            self.add_error('password2', "Ce champ est requis.")
        if username and User.objects.filter(username=username).exists():
            self.add_error('username', "Ce nom d'utilisateur est déjà utilisé.")
        if password and password2 and password != password2:
            self.add_error('password2', "Les mots de passe ne correspondent pas.")

        return cleaned


class LoginForm(forms.Form):
    username = forms.CharField(label="Nom d'utilisateur", max_length=150)
    password = forms.CharField(label="Mot de passe", widget=forms.PasswordInput)

    def clean(self):
        cleaned = super().clean()
        username = cleaned.get('username')
        password = cleaned.get('password')

        if not username:
            self.add_error('username', "Ce champ est requis.")
        if not password:
            self.add_error('password', "Ce champ est requis.")
        if username and password:
            user = authenticate(self.request, username=username, password=password) if hasattr(self, 'request') else authenticate(username=username, password=password)
            if user is None:
                self.add_error(None, "Identifiants incorrects.")
            else:
                cleaned['user'] = user

        return cleaned


class TipForm(forms.ModelForm):
    class Meta:
        model = Tip
        fields = ['content']
        widgets = {
            'content': forms.Textarea(attrs={'rows': 3, 'placeholder': "Votre Life Pro Tip..."})
        }
