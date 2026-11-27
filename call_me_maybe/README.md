# 🤖 Call Me Maybe

**Function calling** (appel de fonctions LLM) par **décodage contraint** sur un
petit LLM local (**Qwen/Qwen3-0.6B**) via le `llm_sdk` fourni.

Le projet traduit une requête naturelle (ex. « What is the sum of 2 and 3? »)
en un **JSON de fonction à appeler** :

```json
{
  "prompt": "What is the sum of 2 and 3?",
  "name": "fn_add_numbers",
  "parameters": {"a": 2.0, "b": 3.0}
}
```

Le modèle **choisit uniquement le nom de la fonction et les valeurs** :
tout le format JSON (accolades, clés, virgules, guillemets) est **écrit
par le programme** grâce au **décodage contraint**.

## Principe : décodage contraint (constrained decoding)

À chaque étape de génération :

1. On obtient les **logits** du modèle pour le token suivant
   (`get_logits_from_input_ids`).
2. Une **machine à états** (FSM) calcule les tokens qui maintiennent un
   JSON **valide** et **conforme au schéma** de la fonction
   (`constraints.py` + `vocab.py`).
3. Les logits des tokens invalides sont mis à **`-inf`**.
4. On prend l'**`argmax`** des logits autorisés.

**Anti-pattern exclu** : pas de « prompting » pour espérer du JSON valide,
pas d'heuristiques pour choisir la fonction. La structure est garantie par
contrainte, le choix du contenu est laissé au modèle.

## Installation

```bash
make install   # uv sync
```

Le `llm_sdk` télécharge le modèle Qwen3-0.6B au premier lancement
(nécessite une connexion internet).

## Utilisation

```bash
make run
# ou
uv run python -m src \
    [--functions_definition data/input/functions_definition.json] \
    [--input data/input/function_calling_tests.json] \
    [--output data/output/function_calling_results.json]
```

La sortie est un tableau JSON avec **exactement** 3 clés par objet :
`prompt`, `name`, `parameters`.

## Tests

```bash
make test      # pytest
make lint      # flake8 + mypy
make lint-strict
```

Les tests utilisent un **vocabulaire factice** et un **mock du modèle**
pour couvrir à 100 % le décodage contraint sans nécessiter le modèle
réel (torch / HuggingFace).

## Structure

```
src/
  __main__.py    entrée python -m src
  cli.py         ligne de commande (arguments, flux, erreurs)
  models.py      modèles pydantic (fonctions, schémas, résultats)
  io_utils.py    lecture/écriture des fichiers JSON
  vocab.py       décodage du vocabulaire BPE (bytes_to_unicode)
  constraints.py machine à états : quels tokens sont autorisés
  decoder.py     décodage contraint : génère un appel valide
  prompting.py   format de prompt Qwen (<tools> / tool_call)
llm_sdk/         SDK fourni (Small_LLM_Model)
data/input/      fonctions_definition.json, function_calling_tests.json
data/output/     résultats (hors Git)
tests/           tests unitaires + eval
```

## Types de paramètres

**Types de base** : `number` (float, virgule requise), `integer`,
`string` (quotes + escapes), `boolean` (`true`/`false`).

**Bonus — types étendus** : `null`, `array` (avec schéma `items`),
`object` (avec schéma `properties`), y compris les structures imbriquées.

## Bonus

- **Types étendus** : `null`, `array`/`object` imbriqués (structure JSON
  forcée par le programme, valeurs générées sous contrainte).
- **Limite de tokens** : `max_tokens` par valeur + `force_close` de
  reprise (anti-débordement).
- **Visualisation** : `--visualize` imprime chaque token généré
  (champ, nombre de tokens autorisés, id, fragment) sur stderr.
- **Multiples modèles** : `--model <id>` pour charger un autre modèle
  Hugging Face (défaut : `Qwen/Qwen3-0.6B`).
- **Timeout** : `--timeout <secondes>` limite le temps par prompt.

```bash
uv run python -m src --model Qwen/Qwen3-1.7B --visualize --timeout 30
```
