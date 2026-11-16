#!/bin/sh
# Exercice 00 : Premier script shell
# Affiche l'adresse reelle vers laquelle renvoie un lien bit.ly.
# Commandes autorisees : curl, grep, cut.
curl -s -I "$1" | grep -i "location" | cut -d " " -f 2
