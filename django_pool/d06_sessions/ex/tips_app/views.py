import random
import time

from django.conf import settings
from django.contrib.auth import login, logout
from django.contrib.auth import get_user_model
from django.http import HttpResponseForbidden
from django.shortcuts import get_object_or_404, redirect, render

from .forms import LoginForm, RegisterForm, TipForm
from .models import CustomUser, Tip

User = get_user_model()


def get_anonymous_name(request):
    """Retourne le nom de session, ou en genere un nouveau apres 42 secondes."""
    now = time.time()
    name = request.session.get('anonymous_name')
    name_set_at = request.session.get('anonymous_name_set_at')

    if name and name_set_at and now - name_set_at < settings.ANONYMOUS_NAME_DURATION:
        return name

    name = random.choice(settings.ANONYMOUS_NAMES)
    request.session['anonymous_name'] = name
    request.session['anonymous_name_set_at'] = now
    return name


def home(request):
    """Page d'accueil : liste des Tips et creation d'un Tip si connecte."""
    anonymous_name = get_anonymous_name(request)

    if request.method == 'POST':
        if not request.user.is_authenticated:
            return redirect('home')
        form = TipForm(request.POST)
        if form.is_valid():
            tip = form.save(commit=False)
            tip.author = request.user
            tip.save()
        return redirect('home')

    tips = Tip.objects.all().order_by('-date')
    context = {
        'anonymous_name': anonymous_name,
        'tips': tips,
        'form': TipForm() if request.user.is_authenticated else None,
        'user': request.user,
        'may_downvote': (request.user.is_authenticated
                         and request.user.has_perm('tips_app.can_downvote')),
        'may_delete': (request.user.is_authenticated
                       and request.user.has_perm('tips_app.delete_tip')),
    }
    return render(request, 'tips_app/home.html', context)


def register(request):
    """Inscription d'un nouvel utilisateur."""
    if request.user.is_authenticated:
        return redirect('home')

    form = RegisterForm(request.POST or None)
    if request.method == 'POST' and form.is_valid():
        User.objects.create_user(
            username=form.cleaned_data['username'],
            password=form.cleaned_data['password'],
        )
        user = User.objects.get(username=form.cleaned_data['username'])
        login(request, user)
        return redirect('home')

    return render(request, 'tips_app/register.html', {
        'form': form,
        'anonymous_name': get_anonymous_name(request),
    })


def login_view(request):
    """Connexion d'un utilisateur."""
    if request.user.is_authenticated:
        return redirect('home')

    form = LoginForm(request, request.POST or None)
    if request.method == 'POST' and form.is_valid():
        login(request, form.cleaned_data['user'])
        return redirect('home')

    return render(request, 'tips_app/login.html', {
        'form': form,
        'anonymous_name': get_anonymous_name(request),
    })


def logout_view(request):
    logout(request)
    return redirect('home')


def vote(request, tip_id, action):
    """Upvote / downvote d'un Tip, avec bascule et permissions."""
    if not request.user.is_authenticated:
        return redirect('home')

    tip = get_object_or_404(Tip, pk=tip_id)

    if action == 'upvote':
        if tip.upvotes.filter(pk=request.user.pk).exists():
            tip.upvotes.remove(request.user)
        else:
            tip.upvotes.add(request.user)
            tip.downvotes.remove(request.user)
    elif action == 'downvote':
        if tip.author != request.user and not request.user.has_perm('tips_app.can_downvote'):
            return HttpResponseForbidden("Vous n'avez pas la permission de downvoter.")
        if tip.downvotes.filter(pk=request.user.pk).exists():
            tip.downvotes.remove(request.user)
        else:
            tip.downvotes.add(request.user)
            tip.upvotes.remove(request.user)
    else:
        return redirect('home')

    tip.author.recompute_reputation()
    return redirect('home')


def delete_tip(request, tip_id):
    """Suppression d'un Tip : auteur ou permission delete_tip."""
    if not request.user.is_authenticated:
        return redirect('home')

    tip = get_object_or_404(Tip, pk=tip_id)
    author = tip.author

    if tip.author != request.user and not request.user.has_perm('tips_app.delete_tip'):
        return HttpResponseForbidden("Vous n'avez pas la permission de supprimer ce tip.")

    tip.delete()
    author.recompute_reputation()
    return redirect('home')
