#!/bin/bash
# Exercice 04 : Virtualenv
# Dossier : ex04/ - Fichiers : requirement.txt, my_script.sh

# Créer un virtualenv nommé django_venv sous python3
python3 -m venv django_venv

# Installer requirement.txt dans le virtualenv
source django_venv/bin/activate
pip install -r requirement.txt

# Le virtualenv reste activé à la fin
