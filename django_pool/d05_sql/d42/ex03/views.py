from django.http import HttpResponse
from django.shortcuts import render
from .models import Movies
from sql_helper import MOVIES_DATA
from datetime import date

def populate(request):
    results = []
    for episode_nb, title, director, producer, release_date_str in MOVIES_DATA:
        try:
            y, m, d = map(int, release_date_str.split('-'))
            Movies.objects.update_or_create(
                episode_nb=episode_nb,
                defaults={
                    'title': title,
                    'director': director,
                    'producer': producer,
                    'release_date': date(y, m, d)
                }
            )
            results.append("OK")
        except Exception as e:
            results.append(f"{title}: {e}")
    return HttpResponse("\n".join(results))

def display(request):
    try:
        movies = Movies.objects.all()
    except Exception:
        return HttpResponse("No data available")
    if not movies:
        return HttpResponse("No data available")
    return render(request, 'ex03/display.html', {'movies': movies})
