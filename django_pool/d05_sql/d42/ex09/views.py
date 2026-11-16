from django.http import HttpResponse
from django.shortcuts import render
from django.core.management import call_command
from .models import Planets, People
import os


def populate(request):
    """Charge les fixtures JSON fournies dans l'app ex09."""
    fixture = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                           'ex09_initial_data.json')
    try:
        call_command('loaddata', fixture, verbosity=0)
        return HttpResponse("OK")
    except Exception as e:
        return HttpResponse(f"Error: {e}", status=500)


def display(request):
    try:
        people = People.objects.select_related('homeworld').filter(
            homeworld__climate__icontains='windy'
        ).order_by('name')
    except Exception:
        return HttpResponse("No data available")
    if not people:
        return HttpResponse("No data available, please use the following command line before use:<br>python3 manage.py loaddata ex09_initial_data.json")
    return render(request, 'ex09/display.html', {'people': people})
