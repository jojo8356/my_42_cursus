from django.shortcuts import render

def django_page(request):
    context = {'title': 'Ex01 : Django, framework web.'}
    return render(request, 'ex01/django.html', context)

def affichage_page(request):
    context = {'title': 'Ex01 : Processus d\'affichage d\'une page statique.'}
    return render(request, 'ex01/affichage.html', context)

def templates_page(request):
    context = {'title': 'Ex01 : Moteur de template.'}
    return render(request, 'ex01/templates.html', context)
