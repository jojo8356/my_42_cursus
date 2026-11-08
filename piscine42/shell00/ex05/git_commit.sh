#!/bin/sh
# Affiche les ids (sha) des 5 derniers commits du depot git courant.
git log --format=%H -5
