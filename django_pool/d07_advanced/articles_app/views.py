from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.mixins import LoginRequiredMixin
from django.shortcuts import redirect
from django.urls import reverse_lazy
from django.utils.translation import gettext_lazy as _
from django.views.generic import (
    CreateView, DeleteView, ListView, DetailView, TemplateView, View
)
from django.contrib.auth.views import LoginView as DjangoLoginView
from django.contrib.auth.views import LogoutView as DjangoLogoutView

from .models import Article, UserFavouriteArticle
from .forms import RegisterForm, ArticleForm, FavouriteForm


class HomeView(TemplateView):
    template_name = 'articles_app/home.html'

    def get(self, request, *args, **kwargs):
        return redirect('articles')


class ArticleListView(ListView):
    model = Article
    template_name = 'articles_app/articles.html'
    context_object_name = 'articles'

    def get_queryset(self):
        return Article.objects.all().order_by('-created')


class ArticleDetailView(DetailView):
    model = Article
    template_name = 'articles_app/article_detail.html'
    context_object_name = 'article'


class PublicationsView(LoginRequiredMixin, ListView):
    model = Article
    template_name = 'articles_app/publications.html'
    context_object_name = 'articles'

    def get_queryset(self):
        return Article.objects.filter(author=self.request.user).order_by('-created')


class FavouritesView(LoginRequiredMixin, ListView):
    model = UserFavouriteArticle
    template_name = 'articles_app/favourites.html'
    context_object_name = 'favourites'

    def get_queryset(self):
        return UserFavouriteArticle.objects.filter(user=self.request.user).select_related('article')


class RegisterView(CreateView):
    form_class = RegisterForm
    template_name = 'articles_app/register.html'
    success_url = reverse_lazy('home')

    def form_valid(self, form):
        form.save()
        # Auto-login après inscription
        username = form.cleaned_data.get('username')
        password = form.cleaned_data.get('password1')
        user = authenticate(self.request, username=username, password=password)
        if user:
            login(self.request, user)
        return super().form_valid(form)


class LoginView(DjangoLoginView):
    template_name = 'articles_app/login.html'
    success_url = reverse_lazy('home')

    def get_success_url(self):
        return reverse_lazy('home')


class LogoutView(DjangoLogoutView):
    success_url = reverse_lazy('home')


class PublishView(LoginRequiredMixin, CreateView):
    model = Article
    form_class = ArticleForm
    template_name = 'articles_app/publish.html'
    success_url = reverse_lazy('publications')

    def form_valid(self, form):
        form.instance.author = self.request.user
        return super().form_valid(form)


class AddFavouriteView(LoginRequiredMixin, CreateView):
    model = UserFavouriteArticle
    form_class = FavouriteForm
    template_name = 'articles_app/add_favourite.html'
    success_url = reverse_lazy('article_detail')

    def get_success_url(self):
        return reverse_lazy('article_detail', args=[self.object.article.pk])

    def get_form_kwargs(self):
        kwargs = super().get_form_kwargs()
        # Préremplir le champ article avec l'article courant
        kwargs['initial'] = {'article': self.kwargs['pk']}
        return kwargs

    def form_valid(self, form):
        form.instance.user = self.request.user
        return super().form_valid(form)
