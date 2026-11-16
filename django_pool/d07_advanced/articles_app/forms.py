from django import forms
from django.contrib.auth.forms import UserCreationForm
from django.contrib.auth.models import User
from .models import Article, UserFavouriteArticle


class RegisterForm(UserCreationForm):
    class Meta:
        model = User
        fields = ['username', 'password1', 'password2']


class ArticleForm(forms.ModelForm):
    class Meta:
        model = Article
        fields = ['title', 'synopsis', 'content']


class FavouriteForm(forms.ModelForm):
    class Meta:
        model = UserFavouriteArticle
        fields = ['article']
