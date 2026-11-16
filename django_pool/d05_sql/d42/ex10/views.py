from django.http import HttpResponse
from django.shortcuts import render
from django.core.management import call_command
from django.db.models import Q
from .models import Planets, People, Movies
import os


def index(request):
    genders = People.objects.values_list('gender', flat=True).distinct()
    genders = [g for g in genders if g]

    results = []
    if request.method == 'POST':
        min_date = request.POST.get('min_date', '')
        max_date = request.POST.get('max_date', '')
        min_diameter = request.POST.get('min_diameter', '0')
        gender = request.POST.get('gender', '')

        try:
            min_diameter = int(min_diameter)
        except ValueError:
            min_diameter = 0

        people_qs = People.objects.filter(gender=gender).filter(
            movies__release_date__gte=min_date,
            movies__release_date__lte=max_date,
            homeworld__diameter__gte=min_diameter
        ).select_related('homeworld').order_by('name').distinct()

        for person in people_qs:
            for movie in person.movies.all():
                results.append({
                    'person_name': person.name,
                    'gender': person.gender,
                    'movie_title': movie.title,
                    'homeworld': person.homeworld.name if person.homeworld else '',
                    'diameter': person.homeworld.diameter if person.homeworld else ''
                })

    return render(request, 'ex10/index.html', {
        'genders': genders,
        'results': results,
    })


def populate(request):
    """Charge les fixtures JSON fournies dans l'app ex10."""
    fixture = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                           'ex10_initial_data.json')
    try:
        call_command('loaddata', fixture, verbosity=0)
        return HttpResponse("OK")
    except Exception as e:
        return HttpResponse(f"Error: {e}", status=500)
