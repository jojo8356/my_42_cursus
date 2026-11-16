#!/bin/bash
# Exercice 01 : Pip
# Dossier : ex01/ - Fichiers : my_script.sh, my_program.py

echo "Version de pip:"
pip --version

echo "Installation de path.py..."
pip install --target=local_lib -e "git+https://github.com/jaraco/path.py.git#egg=path" 2> install.log

if [ -d "local_lib" ]; then
    echo "Installation réussie"
    python3 my_program.py
else
    echo "Échec de l'installation"
fi
