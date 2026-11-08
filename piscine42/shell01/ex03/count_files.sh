#!/bin/sh
# Compte les fichiers ordinaires et repertoires (avec ".") recursivement.
find . \( -type f -o -type d \) | wc -l
