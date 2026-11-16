#!/bin/bash
# Exercice 01 : Pip
# Dossier : ex01/ - Fichiers : my_script.sh, my_program.py
# Installe la version de developpement de path.py dans local_lib puis
# execute my_program.py.

echo "pip version:"
pip --version

# Si la librairie a deja ete installee, l'installation doit l'ecraser.
rm -rf local_lib

pip install --no-build-isolation --target local_lib \
    "git+https://github.com/jaraco/path.py.git#egg=path" > install.log 2>&1

if [ -d local_lib ]; then
    python3 my_program.py
else
    echo "path.py installation failed, see install.log"
    exit 1
fi
