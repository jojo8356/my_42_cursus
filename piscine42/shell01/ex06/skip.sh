#!/bin/sh
# Affiche ls -l une ligne sur deux, en commencant par la premiere.
ls -l | awk 'NR % 2 == 1'
