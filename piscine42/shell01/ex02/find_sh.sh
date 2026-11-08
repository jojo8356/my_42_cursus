#!/bin/sh
# Cherche les fichiers *.sh (recursif) et affiche leur nom sans extension.
find . -name '*.sh' | sed 's|.*/||;s|\.sh$||'
