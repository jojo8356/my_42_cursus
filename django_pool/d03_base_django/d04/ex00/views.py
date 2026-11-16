from django.shortcuts import render

def index(request):
    context = {'title': 'Ex00 : Markdown Cheatsheet.'}
    return render(request, 'ex00/index.html', context)
