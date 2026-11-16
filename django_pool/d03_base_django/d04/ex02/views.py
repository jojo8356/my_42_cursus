import os
from django.shortcuts import render
from django.conf import settings
from datetime import datetime
from .forms import InputForm

def index(request):
    form = InputForm()
    entries = []

    # Lire les logs si le fichier existe
    log_path = settings.LOG_FILE_PATH
    if os.path.exists(log_path):
        with open(log_path, 'r') as f:
            for line in f:
                entries.append(line.strip())

    if request.method == 'POST':
        form = InputForm(request.POST)
        if form.is_valid():
            text = form.cleaned_data['text_input']
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            # Écrire dans le fichier de logs
            with open(log_path, 'a') as f:
                f.write(f"{timestamp}: {text}\n")
            # Recharger les entrées
            with open(log_path, 'r') as f:
                entries = [line.strip() for line in f]

    context = {'form': form, 'entries': entries}
    return render(request, 'ex02/index.html', context)
