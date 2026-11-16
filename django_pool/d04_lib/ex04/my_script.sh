#!/bin/bash
# Exercice 04 : Virtualenv
# Dossier : ex04/ - Fichiers : requirement.txt, my_script.sh

# Créer un virtualenv nommé Django_venv sous python3
python3 -m venv Django_venv

# Installer requirement.txt dans le virtualenv
source Django_venv/bin/activate
pip install -r requirement.txt

# Le virtualenv reste activé à la fin
