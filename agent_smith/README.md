# 🤖 Agent Smith — Agentic AI

Projet 42 — un **agent de codage autonome**. Face à une tâche, il raisonne, écrit
du Python, l'exécute dans un sandbox restreint, observe le résultat et itère
jusqu'à pouvoir soumettre sa réponse :

```
Thought → Code → Observation → Iteration
```

Deux benchmarks sont visés :

- **MBPP** — petits exercices Python autonomes : l'agent écrit une fonction, la
  vérifie via un outil, puis soumet.
- **SWE-bench** — de vraies issues GitHub : l'agent explore un dépôt, localise le
  bug, édite les sources, lance la suite de tests et produit un patch git unifié.

Tout est traçable : chaque run produit un `solution.json` contenant le prompt
système, chaque sortie du modèle, chaque entrée/sortie du sandbox, les tokens et
les temps.

## Structure

```
agent_smith/
├── subject.txt                 # Énoncé complet (obligatoire + bonus)
├── agent_smith.py              # CLI : run / sandbox / evaluate / tools
├── Makefile
├── requirements.txt            # pytest (le cœur est stdlib-only)
├── data/                       # tâches, configs et scripts d'exemple
│   ├── sandbox.json            # SandboxConfig + AgentConfig
│   ├── mbpp_task.example.json
│   ├── swe_task.example.json
│   ├── mbpp_bench.example.json
│   ├── models.example.json     # registre de providers
│   ├── scripted_mbpp.json      # modèle déterministe hors-ligne
│   └── scripted_swebench.json
├── src/
│   ├── models.py               # Task, StepMetrics, SolutionOutput, ExecutionResult
│   ├── config.py               # SandboxConfig, AgentConfig (validation)
│   ├── parsing.py              # extraction de code depuis la réponse du LLM
│   ├── sandbox.py              # exécution restreinte (processus, limites, allowlists)
│   ├── mcp.py                  # registre d'outils + manuel généré (MCP)
│   ├── tools_mbpp.py           # run_tests, check_syntax
│   ├── tools_swebench.py       # les 9 outils obligatoires
│   ├── workspace.py            # backend dépôt (exec, écriture atomique, diff)
│   ├── llm.py                  # providers, rotation de clés, fallback
│   ├── agent_loop.py           # la boucle Thought → Code → Observation
│   ├── evaluator.py            # évaluation de benchmark (pass@1)
│   └── reporting.py            # écriture atomique de solution.json
└── tests/                      # 313 tests
```

## Démarrage

Prérequis : **Python 3.10+**. Aucune dépendance obligatoire (le cœur est stdlib).

```bash
make install          # dépendances de test
make test             # suite complète

# Démos déterministes, sans clé API ni réseau
make run_mbpp         # boucle MBPP complète
make run_swebench     # boucle SWE-bench complète (patch git réel)
make evaluate         # évaluation d'un benchmark (pass@1)
make tools            # liste les outils + manuel généré
make sandbox          # REPL interactif au-dessus du sandbox
```

Avec un vrai modèle :

```bash
python3 agent_smith.py run --benchmark mbpp \
    --task-file data/mbpp_task.example.json --output solution.json \
    --model-name mon-modele --provider-url https://api.example.com/v1
```

Avec un registre multi-providers (`data/models.example.json`) : les clés viennent
uniquement de l'environnement (`GROQ_API_KEY` / `GROQ_API_KEYS`, etc.) ; plusieurs
clés séparées par des virgules sont permutées sur HTTP 429.

## La boucle agent

| Étape | Comportement |
|---|---|
| **Prompt** | prompt système du benchmark + manuel du sandbox, **généré à l'exécution** depuis le serveur MCP connecté |
| **Budget** | l'historique est compacté jusqu'à tenir dans le budget d'entrée restant (marge 10 %), vérifié **avant** chaque requête |
| **Appel** | génération arrêtée sur `<end_code>` / `</tool_call>` : le modèle ne peut pas inventer d'observation |
| **Extraction** | bloc Python, XML Anthropic `<invoke>`, `<tool_call>` Hermes, ReAct `Action:`, bloc non fermé, `Code:` nu — **chaque liberté est renvoyée au modèle** sous forme de `[note]` |
| **Exécution** | stdout, stderr, erreurs, timeouts et troncature deviennent l'observation suivante |
| **Pannes** | rotation de clé sur 429, bascule de modèle avec note de passation, nouvelle tentative complète ensuite |
| **Arrêt** | `final_answer` ou une limite → toujours un `SolutionOutput`, jamais un crash |

## Sandbox

`Sandbox.execute(code)` / `get_manual()` / `close()` — l'agent ne sait jamais quel
serveur est connecté ni quels outils existent.

- **Isolation** : un **processus worker dédié** (pas d'`exec()` in-process), donc
  timeout dur et limite mémoire réellement applicables (`SIGTERM` +
  `resource.setrlimit`).
- **Persistance** : les variables et fonctions survivent d'un bloc à l'autre ;
  après un timeout le worker est remplacé et les blocs réussis sont rejoués.
- **Sécurité** (stdlib uniquement) :
  - imports : `__import__` remplacé par une **allowlist** (motifs `pkg.*`) ;
  - fichiers : `open()` enveloppé, chemins résolus par `realpath` **avant**
    comparaison aux répertoires autorisés (donc `/repo/../etc/passwd` échoue) ;
  - réseau : `socket.socket` lève toujours, et les événements d'audit réseau sont
    refusés ;
  - builtins : allowlist explicite (`eval`, `exec`, `compile`, `input`,
    `breakpoint`, `globals`… retirés) ;
  - **AST avant exécution** : rejet des attributs dunder et des attributs de
    frame (`f_back`, `gi_frame`, `tb_frame`…) ;
  - **hook d'audit** en dernier rempart : voit ce que l'interpréteur fait
    réellement (ouvertures de fichiers, réseau, spawn de processus, `ctypes`) ;
  - `KeyboardInterrupt` / `SystemExit` levés par le code généré ne tuent que le
    worker, jamais l'agent.
- **`final_answer(value)`** est injecté dans chaque espace de noms : ce n'est
  **pas** un outil.
- **Pont MCP** : le client MCP vit dans le **processus parent**, donc un appel
  d'outil n'est pas soumis au timeout du sandbox (un `run_command` peut
  légitimement durer plus longtemps que le code qui l'appelle).

## Outils

**MBPP** : `run_tests(code, test_list=None)` (assertions exécutées une à une dans
un processus séparé, timeout par SIGTERM, première assertion en échec + stdout /
stderr tronqués) et `check_syntax(code)` en extra.

**SWE-bench** — les 9 outils obligatoires :

| Outil | Contrat |
|---|---|
| `read_file` | bornes 1-based incluses, plafond 200 lignes avec indice de pagination, sortie type `cat -n` |
| `edit_file` | remplacement exact, **refuse les correspondances ambiguës** en listant les lignes, écriture atomique, signale un fichier Python qui ne parse plus |
| `list_files` | globs, triés, suffixe `/` pour les dossiers, pagination 100 |
| `search_code` | regex, ignore `.git` et les binaires, pagination 100, `chemin:ligne contenu` |
| `search_function_or_class_definition_in_code` | basé sur l'AST, survit aux fichiers invalides (repli regex) |
| `find_references` | recherche par mot entier, exclut le site de définition |
| `run_command` | stdout, stderr **et code de sortie**, timeout avec sortie partielle |
| `get_patch` | `git -c core.fileMode=false diff` avec intent-to-add, artefacts filtrés |
| `run_tests` | exécute l'`eval_script` (son contenu bash), timeout 600 s |

Deux backends interchangeables : `LocalWorkspace` (checkout local, mode isolation)
et `DockerWorkspace` (un conteneur par tâche, SDK Docker optionnel).

## Tests — **313 tests**

```bash
python3 -m pytest tests/ -v      # ou : make test
```

| Fichier | Couverture |
|---|---|
| `test_models.py` | tâches, résultat d'exécution, métriques, trace |
| `test_config.py` | allowlists, motifs `pkg.*`, anti-traversée, validation, chargement JSON |
| `test_parsing.py` | tous les formats, toutes les notes, fences piégées dans une chaîne |
| `test_sandbox.py` | persistance, timeout, mémoire, troncature, `final_answer`, imports, fichiers, réseau, dunder/frames, builtins, outils, redémarrage |
| `test_tools_mbpp.py` | succès, échec, timeout, syntaxe, exceptions, troncature |
| `test_tools_swebench.py` | les 9 outils, pagination, ambiguïté, écriture atomique, patch, artefacts |
| `test_llm.py` | provider scripté, registre, rotation de clés, fallback |
| `test_agent_loop.py` | succès, 4 types de limites, pannes, compaction, notes, passation |
| `test_mcp.py` | registre, signatures, manuel, invocation, sérialisation |
| `test_evaluator.py` | pass@1, diagnostics, chargement, écriture atomique |
| `test_integration.py` | pipelines MBPP et SWE-bench complets, CLI, fichiers d'exemple |

## Bonus

- Modèle déterministe hors-ligne (`--scripted`) : toute la boucle est testable et
  reproductible **sans clé API ni réseau**.
- Registre multi-providers avec rotation de clés sur 429 et bascule de modèle
  ordonnée par force, avec note de passation.
- Comptabilité tokens/latence par étape et par run.
- Compaction d'historique selon le budget (`fit_view` / `truncate_history`).
- REPL interactif au-dessus du sandbox (`agent_smith.py sandbox`).
- Génération dynamique du manuel d'outils (`agent_smith.py tools`).
- Évaluation de benchmark avec rapport `pass@1` et diagnostics par tâche.
- Trace complète dans `solution.json`, écrite atomiquement.
- Arrêt propre sur SIGINT/SIGTERM : la trace est conservée et tout est fermé.

## Conception

Le cœur (`parsing`, `sandbox`, `mcp`, `tools_*`, `agent_loop`, `evaluator`) est
**pur et injectable** : la boucle ne dépend que de trois fonctions du sandbox, et
le provider est une simple interface. Les implémentations optionnelles (Docker,
OpenAI-compatible) sont chargées paresseusement, avec des équivalents locaux par
défaut (`LocalWorkspace`, `ScriptedProvider`).
