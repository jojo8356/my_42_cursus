# ft_irc — Sujet (énoncé synthétisé le plus complet)

## Contexte
**ft_irc** est un projet de **programmation réseau** à l'école 42 : développer
un **serveur IRC** (Internet Relay Chat) en **C++98**.

## Exigences obligatoires
- Le serveur doit gérer **plusieurs clients simultanément** et **ne jamais
  bloquer**.
- **Un seul `poll()`** (ou équivalent) pour gérer **toutes** les opérations :
  lecture, écriture, et acceptation des connexions.
- Communication **TCP/IP (v4)**.
- Un client IRC doit pouvoir se connecter **sans erreur**.
- Le comportement doit être **similaire à un serveur IRC officiel**.

## Commandes à implémenter
| Commande | Description |
|----------|-------------|
| **CAP LS 302** | Listing des capacités (compatibilité client, ex. HexChat) |
| **PASS** | Mot de passe de connexion (doit précéder NICK/USER) |
| **NICK** | Donne/change le nickname du client |
| **USER** | Spécifie le username et le realname au début de la connexion |
| **JOIN** | Joindre un(ou plusieurs) canal(aux) avec une clé |
| **PART** | Quitter un(ou plusieurs) canal(aux) |
| **MODE** | Définir/retirer des options (modes) sur une cible |
| **TOPIC** | Changer ou visualiser le topic d'un canal |
| **NAMES** | Voir les nicknames membres d'un canal |
| **LIST** | Obtenir la liste des canaux |
| **INVITE** | Inviter un utilisateur dans un canal |
| **KICK** | Demander le retrait forcé d'un utilisateur d'un canal |
| **PRIVMSG** | Envoyer des messages privés entre utilisateurs / aux canaux |
| **NOTICE** | Identique à PRIVMSG, mais aucune réponse automatique |
| **PING** | Tester la présence de la connexion → réponse **PONG** |
| **QUIT** | Fermer la connexion |

## Modes
- **Mode utilisateur** : `i`
- **Modes canaux** : `o, p, s, i, t, n, m, l, b, v, k`
  (implémentation cœur testée : **i, t, k, o, l**)

## Règles de fonctionnement
- Seul le serveur est implémenté (pas de client, pas de serveur-à-serveur).
- RFC 1459 / RFC 2812 comme référence.
- **Tous les messages** envoyés par un client vers un canal doivent être
  retransmis à **chaque autre client** ayant rejoint le canal.
- Il doit y avoir des **opérateurs** et des **utilisateurs réguliers**.
- Les commandes **KICK** et **MODE** (canal) sont réservées aux opérateurs.
- Un canal doit être **détruit** quand son dernier membre part.

## Bonus implémentés
- **OPER** — authentification opérateur serveur.
- **WHOIS** / **WHO** — informations sur les utilisateurs.
- **CAP** — négociation de capacités (LS/REQ/END) pour la compatibilité client.
- **JOIN 0** — quitter tous les canaux.
- **JOIN/PART multi-canaux** (séparés par des virgules).
- **CITEBOT** / **MARVINBOT** — commandes de bot.
- **MOTD** complet (réponses 001-005).
- Gestion propre des **signaux** SIGINT/SIGQUIT (arrêt sans fuite de mémoire).
