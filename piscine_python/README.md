# Piscine Python (42) — Modules 00 à 10

Implémentation complète de la **Piscine Python** de 42, avec **bonus**, validée
par une suite de tests exhaustive : **checker de référence** (tests d'étudiants
de référence, modules 00–08) + **tests maison** pytest (dignés des plus grandes
entreprises, modules 09–10 et régression de structure).

## Résultats

| Méthode de validation | Couverture | Résultat |
|---|---|---|
| Checker de référence `Vince-42/42---python-modules-tester` | Modules 00–08 (44 exercices) | **44/44 PASS** |
| Tests maison pytest | Modules 09–10 + structure 00–10 | **65/65 PASS** |

## Modules

| Module | Thème | Exercices |
|---|---|---|
| 00 | Fondamentaux Python | hello, nom, aire, total, âge, rappel, compte (iter/rec), inventaire |
| 01 | Programmation Objet | intro, données, croissance, factory, sécurité, types, analytique |
| 02 | Exceptions | première, raise, types d'erreurs, erreurs custom, finally |
| 03 | Collections | quête, score, coordonnées, achievements, inventaire, flux, alchimiste |
| 04 | Entrées / Sorties | texte ancien, archive, flux, sécurité coffre |
| 05 | Classes abstraites (ABC) | processeur, flux, pipeline |
| 06 | Import & packages | Alembic (éléments), Distillation (potions), Transmutation, Kaboom (cycle d'import) |
| 07 | Patterns de conception | Factory (battle.py), Mixins (capacitor.py), Stratégie (tournament.py) |
| 08 | Environnement & dépendances | construct, loading, oracle |
| 09 | Validation de données (Pydantic v2) | space_station, alien_contact, space_crew |
| 10 | Programmation fonctionnelle | lambda_spells, higher_magic, scope_mysteries, functools_artifacts, decorator_mastery |

## Structure

```
piscine_python/
├── module00/ … module10/      # chaque module dans exN/ft_*.py (ou scripts principaux)
├── tests/
│   ├── conftest.py            # helpers de chargement / exécution
│   ├── test_module09.py       # tests Pydantic (validators, modèles imbriqués)
│   ├── test_module10.py       # tests fonctionnels (lambdas, clôtures, functools, décorateurs)
│   └── test_structure.py      # régression de l'arborescence 00–10
├── run_tests.py               # runner unifié (pytest)
└── requirements.txt           # dépendances (pandas, numpy, matplotlib, pydantic, dotenv, pytest)
```

## Exécution des tests

```bash
python -m venv venv && source venv/bin/activate
pip install -r requirements.txt
python run_tests.py
```

## Validation de référence (modules 00–08)

```bash
git clone https://github.com/Vince-42/42---python-modules-tester /tmp/pymod_tester
cd /tmp/pymod_tester
python checker.py --student-dir <chemin>/piscine_python/moduleXX --module N
```

## Bonus

- **Module 07** : patterns de conception complets (Factory, Mixins, Stratégie) avec
  classes extensibles et messages de sortie explicites.
- **Module 09** : validators métier inter-champs (`@model_validator`) et modèles
  imbriqués avec règles de collection.
- **Module 10** : décorateurs simples et paramétrés (timer, validator, retry),
  `functools.singledispatch`, `lru_cache`, `partial`, `reduce`.
