from django.http import HttpResponse
from django.shortcuts import render
from .models import Planets, People

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
