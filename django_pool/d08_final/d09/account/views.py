import json

from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.forms import AuthenticationForm
from django.http import JsonResponse
from django.shortcuts import render
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.http import require_POST


def account_page(request):
    """Page account avec AJAX login/logout."""
    form = AuthenticationForm()
    return render(request, 'account/account.html', {'form': form})


@csrf_exempt
@require_POST
def ajax_login(request):
    """AJAX login : POST username/password."""
    form = AuthenticationForm(request, data=request.POST)
    if form.is_valid():
        login(request, form.get_user())
        return JsonResponse({'success': True, 'username': request.user.username})
    return JsonResponse({'success': False, 'errors': form.errors.get_json_data()})


@csrf_exempt
@require_POST
def ajax_logout(request):
    """AJAX logout."""
    logout(request)
    return JsonResponse({'success': True})
