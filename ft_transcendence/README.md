# ft_transcendence

Projet final de 42 — application web full-stack pour un **concours de Pong**.
Implémentation complète du sujet **v3.0.1 (15.12.2024)** avec tous les bonus.

## Stack

- **Backend** : Django 4.2 + Django REST Framework
- **Temps réel** : Django Channels (websockets) — jeu, chat, présence
- **Base de données** : SQLite par défaut (autonome/testable) — PostgreSQL en
  production (activé automatiquement via les variables `POSTGRES_*`)
- **Frontend** : vanilla JavaScript SPA + Bootstrap (toolkit)

## Fonctionnalités (partie obligatoire)

- **Jeu Pong en direct** : deux joueurs, un même clavier (canvas vanilla JS)
- **IA adversaire** : vitesse de raquette identique, rafraîchit sa vue 1/s,
  anticipe les rebonds, **pas d'A***
- **Tournois** : bracket à élimination directe, affichage des matchups/ordre
- **Inscription** : alias au début du tournoi, réinitialisés à chaque tournoi
- **Matchmaking** : file d'attente, annonce du prochain combat
- **Sécurité** : mots de passe hachés (PBKDF2), protection XSS/SQL injection,
  validation des saisies

## Modules (bonus)

- **User management** : inscription, connexion, profil, avatar, amis, statut
  en ligne, statistiques victoires/défaites, historique de matchs
- **OAuth 2.0 avec 42** : flux d'autorisation complet (provider configurable
  + mock pour les tests)
- **JWT** : access + refresh, **logout** avec blacklist des tokens
- **2FA** : authentification à deux facteurs (secret TOTP)
- **Remote players** : synchronisation du jeu entre joueurs distants via websockets
- **Live Chat** : DM, canaux (public/privé/à mot de passe), blocage, présence,
  invitations à jouer
- **Game customization** : bonus (vitesse/taille de raquette, balle lente)
- **Multilingue** : EN / FR / DE + sélecteur
- **RGPD** : anonymisation, suppression de compte
- **Tournois + matchmaking + historique**

## Structure

```
ft_transcendence/
├── manage.py
├── requirements.txt
├── subject.txt          # énoncé complet v3.0.1
├── config/              # settings, urls, asgi (Channels), wsgi
├── accounts/            # User, JWT, OAuth 42, profil, amis, chat DM, RGPD, 2FA
├── games/               # moteur Pong, IA, sessions, personnalisation
├── tournaments/         # bracket, matchmaking, modèles
├── chat/                # canaux, messages, consumer websocket
├── frontend/            # SPA vanilla JS + Bootstrap (index.html, app.js, css)
└── tests/               # suite complète (147 tests)
```

## Installation & exécution

```bash
make install      # pip install -r requirements.txt
make migrate      # migrer la base
make run          # lancer le serveur (0.0.0.0:8000)
```

Le serveur serve l'SPA à `/` et l'API sous `/api/`.

## Tests

```bash
make test         # = python tests/run_tests.py
```

Suite de **147 tests** passant, couvrant :
- Moteur Pong (physique, rebonds, collisions, scoring, bonus, déterminisme)
- IA (refresh 1/s, vitesse identique, anticipation, pas d'A*)
- Bracket tournois + matchmaking
- API accounts / games / tournaments / chat
- Sécurité (SQL injection, XSS, hachage, validation, authentification)
- Patterns de grands étudiants (profil public cache l'email, cycle d'amis,
  strength de mot de passe, logout/refresh)

## Points clés de sécurité

- Mots de passe : PBKDF2 (validators Django, mots de passe numériques rejetés)
- Tokens JWT signés (HS256), logout avec blacklist
- XSS : échappement des messages côté serveur
- SQL injection : paramétrage des requêtes (pas de concaténation)
- Sensibilisation : `.env` ignoré par git, aucune credential commitée
