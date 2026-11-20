# Philosophers

Projet **Philosophers** de 42 School (énoncé officiel **v12.0**).

Simulation de philosophes autour d'une table qui mangent des spaghettis avec
deux fourchettes. Deux implémentations sont fournies :

| Partie     | Dossier        | Binaire        | Technologie                |
|------------|----------------|----------------|----------------------------|
| Obligatoire | `philo/`       | `philo`        | Threads + mutex (pthread)  |
| Bonus       | `philo_bonus/` | `philo_bonus`  | Processus + sémaphores nommés |

## Arguments

```
number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

- `number_of_philosophers` : nombre de philosophes (= nombre de fourchettes).
- `time_to_die` : en ms. S'un philosophe n'a pas commencé à manger dans ce
  délai depuis son dernier repas (ou le début de la simulation), il meurt.
- `time_to_eat` : durée du repas (ms).
- `time_to_sleep` : durée du sommeil (ms).
- `number_of_times_each_philosopher_must_eat` (optionnel) : quand tous les
  philosophes ont atteint ce nombre de repas, la simulation s'arrête.

## Logs

Chaque changement d'état est affiché au format :

```
timestamp_in_ms X <message>
```

Messages autorisés : `has taken a fork`, `is eating`, `is sleeping`,
`is thinking`, `died`.

- Un message n'est jamais écrasé/entrelacé avec un autre.
- La mort est annoncée dans un délai **≤ 10 ms**.
- Pas de data races, pas de variables globales, pas de libft.

## Build

```bash
make -C philo          # compile philo
make -C philo bonus    # compile philo_bonus (règle bonus)
make -C philo_bonus    # compile philo_bonus
make -C philo re       # rebuild complet
make -C philo fclean   # nettoyage binaire + objets
```

Compile avec `cc -Wall -Wextra -Werror`.

## Exécution

```bash
./philo/philo 3 2000 200 200 2
./philo_bonus/philo_bonus 3 2000 200 200 2
```

## Architecture

### Partie obligatoire (`philo/`)

- **Un thread par philosophe**, **un mutex par fourchette**.
- Cycle : *penser* → prendre deux fourchettes → *manger* → dormir.
- Les fourchettes sont prises dans un ordre fixe (indice le plus bas en
  premier) pour éviter l'interblocage (deadlock).
- Le **monitor** (thread dédié) vérifie toutes les 1 ms si un philosophe
  n'a pas mangé depuis `time_to_die` ; il annonce la mort et lance l'arrêt.
- `increment_meals` détecte la fin quand tous les philosophes ont atteint
  leur nombre de repas.

Fichiers : `philo.h`, `philo.c` (main + init/cleanup), `utils.c` (temps,
logs, `ft_atoi` maison), `philosopher.c` (routine du thread), `monitor.c`
(détection des morts).

### Partie bonus (`philo_bonus/`)

- **Tous les fourchettes au milieu de la table** : le nombre de fourchettes
  disponibles est un **sémaphore nommé** (`sem_open`, valeur initiale =
  nombre de philosophes).
- **Un processus par philosophe** (`fork`) ; **le processus principal est
  uniquement le monitor** et ne mange pas.
- La mémoire partagée (`mmap MAP_SHARED`) stocke `last_meal_time[]` et
  `meals_eaten[]` pour que le monitor lise l'état des enfants.
- Le philosophe prend des fourchettes avec `sem_wait`, mange, relâche avec
  `sem_post`.
- Le monitor détecte l'inanition (pas de repas depuis `time_to_die`), log
  `died`, tue les enfants (`kill SIGKILL`) et récupère avec `waitpid`.
- `sem_unlink` nettoie le sémaphore à la fin (pas de résidu).

Fichiers : `philo_bonus.h`, `philo_bonus.c` (main, mmap, fork), `utils_bonus.c`,
`philosopher_bonus.c` (processus enfant), `monitor_bonus.c` (parent).

## Tests

Suite de tests complets (Python) dans `tests/test_philo.py` :

```bash
python3 tests/test_philo.py
```

Vérifie :
- compilation stricte (`-Wall -Wextra -Werror`),
- validation des arguments (args invalides → sortie 1),
- format des logs (timestamp + id + message autorisé),
- absence de chevauchement / messages tronqués,
- absence de blocage (tous les scénarios terminent),
- précision du décès (≤ 10 ms),
- achèvement des repas (arrêt propre),
- tests de stress (25 itérations par partie) pour détecter deadlock / races,
- nettoyage du sémaphore (pas de résidu),
- absence de processus zombies.

Résultat : **87/87 tests passés**. La partie obligatoire est également
vérifiée sans data race sous **ThreadSanitizer**.
