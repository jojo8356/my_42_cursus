# webserv — Sujet

## Objectif

Développer un serveur web **HTTP/1.1** en **C++98**, inspiré de **NGINX**, capable
de gérer **plusieurs clients simultanés** via une **boucle d'événements unique**
(`select()`), sans thread par connexion.

## Architecture

- **1 boucle d'événements** (`select()`) gérant :
  - les sockets d'écoute
  - les connexions clients (non-bloquantes)
  - les pipes de communication avec les processus CGI
- **Pas de thread par client** — tout est géré dans le même thread.
- **Sockets non-bloquants**.

## Configuration (fichier NGINX-style)

Le serveur lit un fichier de configuration avec des blocs `server` et `location` :

| Directive | Description |
|-----------|-------------|
| `server` | Bloc de serveur (peut être multiple, ports différents) |
| `location` | Bloc de routage par préfixe |
| `listen` | `host:port` d'écoute |
| `server_name` | Nom du serveur |
| `root` | Racine des fichiers statiques |
| `alias` | Chemin alternatif (remplacement du préfixe) |
| `index` | Fichier(s) d'index d'un répertoire |
| `error_page` | Pages d'erreur personnalisées |
| `client_max_body_size` | Taille maximale du corps de requête |
| `allow_methods` / `limit_except` | Méthodes HTTP autorisées |
| `autoindex` | Génération de liste de répertoire |
| `cgi` / `cgi_pass` / `cgi_ext` | Configuration CGI (interprète par extension) |
| `upload_store` | Répertoire d'upload |
| `return` | Redirection (`return 301 <url>`) |

Le routage des `location` utilise le **préfixe le plus long**.

## Méthodes HTTP

- **GET** — récupération de ressources
- **POST** — envoi de données (formulaires, uploads)
- **DELETE** — suppression de ressources
- **HEAD** — headers seulement

## Fonctionnalités

### Fichiers statiques
- Servir des fichiers avec les **bons types MIME**.
- **Streaming** des fichiers.
- **Index** de répertoire (`index`).
- **Listing de répertoire** (`autoindex`).

### CGI
- Exécution de scripts via `fork()` + pipes.
- **Variables d'environnement RFC 3875** : `REQUEST_METHOD`, `QUERY_STRING`,
  `CONTENT_TYPE`, `CONTENT_LENGTH`, `PATH_INFO`, `SCRIPT_FILENAME`,
  `HTTP_COOKIE`, `SERVER_PROTOCOL`, `GATEWAY_INTERFACE`, etc.
- Scripts **multiples** : `.py`, `.pl`, ...
- **Timeout CGI** → **504 Gateway Timeout**.
- Erreur de script → **502 Bad Gateway**.

### Uploads
- **POST multipart** → enregistrement dans `upload_store` (réponse **201**).
- Fichier déjà existant → **409**.
- **DELETE** pour supprimer → **204**.
- Corps trop grand → **413 Payload Too Large**.

### Redirections
- `return 301 <url>` → **301 Moved Permanently** avec header `Location`.

### Pages d'erreur
- **404 Not Found**, **403 Forbidden**, **400 Bad Request**, **405 Method Not
  Allowed**, **413 Payload Too Large**, **500 Internal Server Error**,
  **501 Not Implemented**, **502 Bad Gateway**, **504 Gateway Timeout**.
- Pages personnalisables via `error_page`.

### Corps de requête
- `Content-Length` **et** `Transfer-Encoding: chunked`.
- Dépassement de `client_max_body_size` → **413**.

### Keep-alive
- Connexions persistantes HTTP/1.1.

## Bonus

- **Sessions / Cookies** — cookie `webservsid`.
- **Multiples types CGI** (Python, Perl, ...).
- **Header `Date`** sur chaque réponse.
- **Logs** — `access.log` (accès) et `error.log` (erreurs).
- **Timeouts** — **408 Request Timeout** (slowloris).
- **Multiples blocs `server`** — ports différents.

## Validation

- Compilation : `make` (C++98 strict, `-Wall -Wextra -Werror`).
- Tests : `python3 tests/run_tests.py` (harness stdlib Python, requêtes HTTP réelles).
