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

    def clean(self):
        cleaned = super().clean()
        user = self.instance.user
        article = cleaned.get('article')
        if user and article and UserFavouriteArticle.objects.filter(
                user=user, article=article).exists():
            self.add_error('article', 'This article is already in your favourites.')
        return cleaned
