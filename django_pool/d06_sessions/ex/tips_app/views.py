import random
import time
from datetime import datetime, timedelta

from django.conf import settings
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.decorators import login_required
from django.contrib.auth.mixins import LoginRequiredMixin
from django.http import HttpResponse, HttpResponseForbidden, HttpResponseRedirect
from django.shortcuts import render, get_object_or_404, redirect
from django.urls import reverse
from django.views import View

from .forms import RegisterForm, LoginForm, TipForm
from .models import Tip


def get_anonymous_name(request):
    """Récupère ou génère le nom anonyme de l'utilisateur."""
    now = time.time()
    name = request.session.get('anonymous_name')
    name_set_at = request.session.get('anonymous_name_set_at')

    if name and name_set_at and now - name_set_at < settings.ANONYMOUS_NAME_DURATION:
        return name

    # Générer un nouveau nom
    name = random.choice(settings.ANONYMOUS_NAMES)
    request.session['anonymous_name'] = name
    request.session['anonymous_name_set_at'] = now
    return name


def home(request):
    """Page d'accueil avec le système de sessions anonymes et les Tips."""
    # Récupérer le nom anonyme
    anonymous_name = get_anonymous_name(request)

    # Liste des tips
    tips = Tip.objects.all().order_by('-date')

    form = TipForm()

    context = {
        'anonymous_name': anonymous_name,
        'tips': tips,
        'form': form,
        'user': request.user,
    }

    return render(request, 'tips_app/home.html', context)


def register(request):
    """Page d'inscription."""
    if request.user.is_authenticated:
        return redirect('home')

    form = RegisterForm(request.POST or None)

    if request.method == 'POST' and form.is_valid():
        username = form.cleaned_data['username']
        password = form.cleaned_data['password']
        from django.contrib.auth.models import User
        user = User.objects.create_user(username=username, password=password)
        login(request, user)
        return redirect('home')

    anonymous_name = get_anonymous_name(request)
    return render(request, 'tips_app/register.html', {
        'form': form,
        'anonymous_name': anonymous_name,
    })


def login_view(request):
    """Page de connexion."""
    if request.user.is_authenticated:
        return redirect('home')

    form = LoginForm(request.POST or None)

    if request.method == 'POST' and form.is_valid():
        user = form.cleaned_data['user']
        login(request, user)
        return redirect('home')

    anonymous_name = get_anonymous_name(request)
    return render(request, 'tips_app/login.html', {
        'form': form,
        'anonymous_name': anonymous_name,
    })


def logout_view(request):
    """Déconnexion."""
    logout(request)
    return redirect('home')


@login_required
def create_tip(request):
    """Créer un Tip (seulement si connecté)."""
    if request.method == 'POST':
        form = TipForm(request.POST)
        if form.is_valid():
            tip = form.save(commit=False)
            tip.author = request.user
            tip.save()
            return redirect('home')
    return redirect('home')


@login_required
def vote(request, tip_id, action):
    """Vote up/down sur un Tip."""
    tip = get_object_or_404(Tip, pk=tip_id)

    if action == 'upvote':
        if tip.upvotes.filter(pk=request.user.pk).exists():
            tip.upvotes.remove(request.user)
        else:
            tip.upvotes.add(request.user)
            tip.downvotes.remove(request.user)
    elif action == 'downvote':
        # Permission personnalisée : l'auteur peut downvoter, ou réputation >= 15
        if tip.author != request.user and not request.user.has_perm('tips_app.can_downvote'):
            return HttpResponseForbidden("Vous n'avez pas la permission de downvoter.")
        if tip.downvotes.filter(pk=request.user.pk).exists():
            tip.downvotes.remove(request.user)
        else:
            tip.downvotes.add(request.user)
            tip.upvotes.remove(request.user)

    return redirect('home')


@login_required
def delete_tip(request, tip_id):
    """Supprimer un Tip."""
    tip = get_object_or_404(Tip, pk=tip_id)

    # L'auteur peut supprimer son tip, ou l'utilisateur a la permission
    if tip.author != request.user and not request.user.has_perm('tips_app.can_delete'):
        return HttpResponseForbidden("Vous n'avez pas la permission de supprimer ce tip.")

    tip.delete()
    return redirect('home')


@login_required
def create_tip_view(request):
    """Vue pour créer un Tip."""
    if request.method == 'POST':
        form = TipForm(request.POST)
        if form.is_valid():
            tip = form.save(commit=False)
            tip.author = request.user
            tip.save()
            return redirect('home')
    return redirect('home')
