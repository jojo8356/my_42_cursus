#!/bin/sh
# Exercice 00 : Premier script shell
# Affiche l'adresse réelle d'une adresse bit.ly
# Fonctions autorisées : curl, grep, cut

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <bit.ly_url>" >&2
    exit 1
fi

URL="$1"

# curl récupère la redirection, grep extrait l'URL réelle, cut nettoie
RESULT=$(curl -s -o /dev/null -w '%{redirect_url}' "$URL")

if [ -z "$RESULT" ]; then
    echo "Impossible de récupérer l'URL réelle" >&2
    exit 1
fi

echo "$RESULT"
