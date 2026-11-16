# Piscine Django 42 School

Implémentation complète de la piscine Django de 42 School, basée sur le sujet officiel (repo `Tednoob17/42`).

## Modules implémentés

| Module | Dossier | Sujet |
|--------|---------|-------|
| d00 Initiation | `d00_initiation/` | HTML, CSS, JavaScript, Web |
| d01 Starting | `d01_starting/` | Python de base (variables, dictionnaires, fichiers, POO) |
| d02 Oob | `d02_oob/` | POO Python (classes, héritage, exceptions) |
| d03 Base Django | `d03_base_django/` | Projet Django de base (projet, apps, templates, forms) |
| d04 Lib | `d04_lib/` | Librairies Python (geohash, path, API Wikipedia, virtualenv, Hello World) |
| d05 SQL | `d05_sql/` | ORM et SQL Django (models, psycopg2, tables, foreign keys, many-to-many) |
| d06 Sessions | `d06_sessions/` | Sessions, utilisateurs, permissions, réputation |
| d07 Advanced | `d07_advanced/` | Vues génériques, template tags, Bootstrap, i18n, tests |
| d08 Final | `d08_final/` | AJAX et WebSockets (login AJAX, chat) |

## Comment lancer les projets Django

### d03 Base Django
```bash
cd d03_base_django/d04
pip install -r requirements.txt
python manage.py migrate
python manage.py runserver
```

### d05 SQL
```bash
cd d05_sql/d42
pip install -r requirements.txt
python manage.py migrate
python manage.py runserver
```
Note : le sujet exige PostgreSQL. Pour les tests, SQLite est utilisé en fallback.
Pour PostgreSQL, modifier `d42/settings.py` et installer psycopg2.

### d06 Sessions
```bash
cd d06_sessions/ex
pip install -r requirements.txt
python manage.py migrate
python manage.py createsuperuser
python manage.py runserver
```

### d07 Advanced
```bash
cd d07_advanced
pip install -r requirements.txt
python manage.py migrate
python manage.py runserver
```

### d08 Final
```bash
cd d08_final/d09
pip install -r requirements.txt
python manage.py migrate
python manage.py runserver
```
Pour les WebSockets, utiliser Daphne :
```bash
python manage.py runserver  # ou daphne d09.asgi:application
```

## Exercices Python (d00-d04)

Les exercices Python purs (d00 HTML, d01 Starting, d02 Oob, d04 Lib) ne nécessitent que Python 3 :
```bash
cd d01_starting/ex00
python3 var.py
```
