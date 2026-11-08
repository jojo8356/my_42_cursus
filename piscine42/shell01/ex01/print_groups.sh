#!/bin/sh
# Affiche les groupes de l'utilisateur $FT_USER, separes par des virgules.
id -Gn "$FT_USER" | tr ' ' ','
