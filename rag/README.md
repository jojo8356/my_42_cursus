# 📚 RAG — RAG Against the Machine

Projet 42 — Retrieval-Augmented Generation sur un dépôt de code. Transformer
un dépôt en base de connaissances interrogeable : **indexation → récupération
→ augmentation → génération**, avec des réponses ancrées et traçables.

Implémentation en **Python 3 pur** (cœur sans I/O ni dépendances lourdes), avec
des abstractions optionnelles pour des embeddings/LLM réels (OpenAI,
sentence-transformers, FAISS, FastAPI).

## Structure

```
rag/
├── subject.txt            # Énoncé complet (partie + bonus)
├── rag.py                 # Point d'entrée CLI (index/search/answer/evaluate/api/interface)
├── requirements.txt       # pytest (le cœur utilise la stdlib)
├── Makefile               # install / index / search / answer / evaluate / api / test
├── data/config.ini.example
├── src/
│   ├── models.py          # Chunk, SearchResult, Answer, ManifestEntry (données pures)
│   ├── chunker.py         # AST Python / Markdown / texte + enrichissement chemin + offsets exacts
│   ├── bm25.py            # Moteur BM25 + bonus symboles/docs + expansion + cache
│   ├── embedder.py        # HashingEmbedder (local, déterministe) + OpenAIEmbedder (optionnel)
│   ├── vector_store.py    # Stockage vectoriel cosinus + persistance atomique
│   ├── rrf.py             # Reciprocal Rank Fusion (recherche hybride)
│   ├── retriever.py       # BM25 / sémantique / hybride + fallback gracieux
│   ├── generator.py       # Génération ancrée + anti-hallucination (local + OpenAI optionnel)
│   ├── indexer.py         # Ingestion + manifeste + indexation incrémentale + caches
│   ├── evaluator.py       # Recall@k
│   ├── config.py          # config.ini persistée
│   ├── api.py             # API HTTP FastAPI (bonus, optionnel)
│   └── interface.py       # Interface terminale interactive curses (bonus)
└── tests/                 # Suite de tests (pytest)
```

## Démarrage

```bash
pip install -r requirements.txt

# Indexer un dépôt (BM25)
python3 rag.py --repo /chemin/du/repo --work-dir data index

# Indexer avec récupération sémantique (hybride)
python3 rag.py --repo /chemin/du/repo --work-dir data index --hybrid

# Rechercher
python3 rag.py --repo /chemin/du/repo --work-dir data search "allocate_slots" --k 5

# Générer une réponse ancrée
python3 rag.py --repo /chemin/du/repo --work-dir data answer "que fait allocate_slots ?"

# Évaluer (Recall@k)
python3 rag.py evaluate --results results.json --dataset dataset.json

# API HTTP (bonus)
python3 rag.py api

# Interface terminale interactive (bonus)
python3 rag.py interface
```

## Tests

```bash
python3 -m pytest tests/ -v
#   ou : make test
```

La suite couvre : le chunking (AST/Markdown/texte/symboles/localisations exactes),
le BM25 (indexation, classement, bonus, expansion, persistance), les embeddings
et le store vectoriel (cosine, persistance atomique), le RRF hybride, le
récupérateur (trois modes + fallback), la génération (ancrée, anti-hallucination),
l'indexation incrémentale (manifest, hash, rechargement, caches embeddings),
l'évaluation Recall@k, la configuration, et des scénarios d'intégration de
bout en bout (index → search → answer → evaluate).

## Fonctionnalités

**Obligatoire** : ingestion multi-fichiers, chunking adaptatif (AST Python,
hiérarchie Markdown, texte brut), récupération **BM25**, enrichissement par
chemin, **localisations exactes** (fichier + offsets caractères), génération
ancrée, évaluation **Recall@k**, CLI.

**Bonus** : récupération **sémantique** (embeddings + cosinus), recherche
**hybride** (RRF), **indexation incrémentale** (manifest + hash), **caches
persistents** (embeddings + requêtes), **API HTTP** (FastAPI), **interface
terminale interactive** (curses), **configuration persistée**, **fallback BM25**
automatique, **symboles AST**, **expansion de requête**, **bonus documentation**,
**anti-hallucination** (consigne + seuil de pertinence).

## Conception

Le moteur (chunker, BM25, retriever, generator, indexer, evaluator) est **pur
et sans I/O**, entièrement déterministe et testable sans réseau ni modèles
lourds. Les implémentations réelles (OpenAI embedder/generator, FAISS, FastAPI)
sont optionnelles et importées laziment, avec des implémentations locales
équivalentes par défaut (hashing embedder, générateur extractif, store vectoriel
pur Python).
