#!/bin/sh
# Affiche les adresses MAC de la machine, une par ligne.
ifconfig 2>/dev/null | grep -o -E '([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}'
