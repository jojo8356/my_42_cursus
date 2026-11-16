# Harnais de tests — piscine Django

Suite de tests automatisés couvrant les 9 jours de la piscine Python-Django
(`d00_initiation` … `d08_final`). Les tests sont issus des sujets officiels et
des tests disponibles dans la communauté, adaptés pour être exécutés sans
poste de soutenance (pas de PostgreSQL, pas de réseau, pas de navigateur).

## Lancer tout le harnais

```bash
python3 django_pool/tests/run_all.py
```

## Lancer un jour

```bash
cd django_pool/tests
python3 check_d01.py
```

Chaque `check_dNN.py` affiche un rapport `PASS / FAIL` et sort avec le code
`0` si tout est vert, `1` sinon.

## Résultats attendus (tous les exercices corrigés)

| Jour | Projet testé | Tests |
|------|--------------|-------|
| d00 | `d00_initiation` | 70 |
| d01 | `d01_starting` | 30 |
| d02 | `d02_oob` | 28 |
| d03 | `d03_base_django/d04` | 24 |
| d04 | `d04_lib` | 20 |
| d05 | `d05_sql/d42` | 29 |
| d06 | `d06_sessions/ex` | 59 |
| d07 | `d07_advanced` | 51 |
| d08 | `d08_final/d09` | 29 |

## Organisation

- `_harness.py` — utilitaires communs (rapport, execution de sous-processus,
  repertoires temporaires, normalisation).
- `check_dNN.py` — un fichier par jour ; chaque fichier est autonome.
- `fixtures/` — ressources officielles (PDF/CSV/JS/CSS/HTML de reference)
  utilisees pour comparer les rendus aux attentes des sujets.
- `official/` — le seul test fourni avec les sujets (`d02/ex04/tests.py`).
- `community/` — tests trouves dans des depot GitHub d'autres candidats ;
  certains sont juges non fiables (voir ci-dessous) et ne sont pas utilises
  comme reference d'acceptation.

## Particularites d'execution

- Les projets Django (`d03`, `d05`, `d06`, `d07`, `d08`) tournent sur **SQLite**
  et une base temporaire, via la variable d'environnement `DJANGO_DB_PATH`.
  Aucun PostgreSQL n'est necessaire (les exercices SQL restent ecrits pour
  PostgreSQL en soutenance ; un `sql_helper.py` bascule sur SQLite pour
  permettre la validation).
- `d00/ex00` utilise un petit serveur HTTP local qui repond `302` pour
  verifier la resolution d'une redirection.
- `d04/ex00` (geohashing) est teste avec un module `antigravity` de
  substitution, car le paquet PyPI ne fournit pas `geohash`.
- `d04/ex02` et `d04/ex03` sont testes avec un faux module `requests` pour
  eviter tout appel reseau.
- `d08` teste le consumer WebSocket avec `channels.testing.WebsocketCommunicator`
  et une channel layer en memoire.
- `d07/ex06` execute `manage.py test articles_app` (les tests ecrits pour
  l'exercice).

## Tests communautaires ecartes

- `d02/ex03` : appele `machine.serve()` sans `repair()`, ce qui leve une
  exception non geree ; le test est biaise.
- `d02/ex05` : attend des guillemets non echappes (contredit le test officiel
  `d02/ex04`) et utilise `traceback` sans l'importer.

Ces deux tests sont remplaces dans `check_d02.py` par des tests ecrits a
partir du sujet et du test officiel.
