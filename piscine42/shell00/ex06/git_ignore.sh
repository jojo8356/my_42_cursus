#!/bin/sh
# Liste les fichiers ignores par le depot git courant.
git ls-files -o -i --exclude-standard
