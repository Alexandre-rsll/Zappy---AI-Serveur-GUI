# Zappy

**Projet Epitech — 2ème année | Note obtenue : A**

Zappy est un jeu multijoueur en réseau où plusieurs équipes s'affrontent sur une carte remplie de ressources. Chaque joueur, piloté par une IA autonome, explore le terrain, se nourrit et récolte des pierres pour évoluer via un rituel d'élévation collectif. L'équipe gagnante est la première à faire atteindre le niveau maximum à six de ses joueurs.

Le projet impose de concevoir, en équipe, trois programmes indépendants qui communiquent uniquement via un protocole réseau texte : un serveur de jeu, un client graphique 3D et une intelligence artificielle. Il met l'accent sur la conception réseau bas niveau, l'architecture logicielle multi-binaires et la conception d'agents autonomes.

## Sommaire

- [Architecture](#architecture)
- [Serveur (C++)](#serveur-c)
- [Interface graphique (C++ / raylib)](#interface-graphique-c--raylib)
- [Intelligence artificielle (Python)](#intelligence-artificielle-python)
- [Bonus : agent par apprentissage par renforcement](#bonus--agent-par-apprentissage-par-renforcement)
- [Stack technique](#stack-technique)
- [Compilation et lancement](#compilation-et-lancement)
- [Qualité et intégration continue](#qualité-et-intégration-continue)
- [Équipe](#équipe)

## Architecture

Le dépôt est organisé en modules indépendants, chacun compilé comme une bibliothèque CMake distincte et réutilisée par les deux binaires C++ :

```
.
├── server/          binaire zappy_server
├── gui/              binaire zappy_gui
├── ai/                client IA de référence (Python)
├── bonus/            agent IA par apprentissage par renforcement
└── libs/
    ├── network_server/  boucle d'événements et gestion des sockets (poll)
    ├── protocol/          encodage/décodage du protocole joueur <-> serveur
    ├── gui_protocol/     protocole dédié serveur <-> GUI et état du monde
    ├── game/               logique de jeu (carte, ressources, élévation)
    ├── gui_client/         rendu 3D, caméra, interactions (raylib)
    ├── cli/                  parsing des options en ligne de commande
    └── logger/            journalisation applicative
```

Cette séparation permet de tester chaque composant indépendamment et de réutiliser le protocole et le CLI entre le serveur et le client graphique.

## Serveur (C++)

- Processus **mono-thread, événementiel**, construit autour d'un unique appel à `poll()` : aucune attente active, aucun blocage tant qu'un socket n'a pas d'événement à traiter.
- Gère en parallèle un nombre arbitraire de clients IA et de clients graphiques, avec une file de commandes par client (max 10 commandes en attente, exécutées dans l'ordre de réception).
- Implémente l'intégralité des règles du jeu : génération et redistribution périodique des ressources, calcul du champ de vision par niveau, propagation directionnelle des sons (`broadcast`), reproduction (`fork`/éclosion des œufs), éjection de joueurs, et vérification du rituel d'élévation (quorum de joueurs + ressources requises, vérifié au début **et** à la fin de l'incantation).
- Le monde (`World`) est représenté par une grille torique (sortie à droite = entrée à gauche) de tuiles contenant ressources, joueurs et œufs.

## Interface graphique (C++ / raylib)

- Client graphique **3D** développé avec [raylib](https://www.raylib.com/) / raylib-cpp (récupérés via `FetchContent`), avec shaders de lighting personnalisés (GLSL) et modèles `.gltf` animés par niveau d'évolution des joueurs.
- Se connecte au serveur en s'authentifiant comme client spécial `GRAPHIC`, via un protocole dédié (`gui_protocol`) distinct de celui des IA, optimisé pour ne pousser que les changements de tuiles pertinents (delta updates plutôt que resynchronisation complète).
- Caméra libre, sélection/interaction avec les entités du monde, et panneau de logs affichant les évènements de la partie en temps réel.

## Intelligence artificielle (Python)

Client de référence pilotant un joueur de façon totalement autonome, architecturé autour d'un **arbre de comportement** (behaviour tree à base de `sequence` / `selector` / `condition`) combiné à un système de **rôles** dynamiques :

- `survivor` — priorité à la survie (seuil de nourriture critique) ;
- `collector` — exploration et récolte de ressources ;
- `forker` — gestion de la reproduction pour ouvrir de nouveaux slots d'équipe ;
- `leader` — coordination des incantations collectives (rassemblement, patience, cooldown).

Un module de **coordination d'équipe** encode des messages compacts sur le canal `broadcast` du jeu (portée et bande passante limitées par le protocole) pour que les bots se recrutent mutuellement et synchronisent les élévations sans canal de communication hors-jeu. Chaque bot maintient sa propre carte mentale du monde (`mental_map`), son inventaire et son champ de vision, et communique avec le serveur via une file de commandes asynchrone respectant la contrainte des 10 requêtes en vol.

## Bonus : agent par apprentissage par renforcement

En complément de l'IA à base de règles, le dossier `bonus/` contient un pipeline complet d'**apprentissage par renforcement** (`train_rl.py` / `eval_rl.py`) qui entraîne une politique en faisant jouer les bots en self-play contre le vrai binaire serveur (lancé en sous-processus), avant de l'évaluer face à l'IA classique.

## Stack technique

| Composant | Langage / outils |
|---|---|
| Serveur | C++20, CMake, `poll()` (I/O multiplexing) |
| Client graphique | C++20, raylib / raylib-cpp, GLSL, glTF |
| IA | Python 3 |
| Bonus IA | Python 3, apprentissage par renforcement |
| Qualité | clang-format, clang-tidy, GitHub Actions (self-hosted runner) |

## Compilation et lancement

```bash
./compile.sh          # build incrémental
./compile.sh re        # rebuild complet
./compile.sh d          # build debug

./zappy_server -p 4242 -x 20 -y 20 -n TeamA TeamB -c 6 -f 100
./zappy_gui -p 4242 -h localhost
python3 -m zappy_ai -p 4242 -n TeamA -h localhost
```

## Qualité et intégration continue

Chaque push déclenche une pipeline CI qui vérifie le formatage (`clang-format`), l'analyse statique (`clang-tidy`) et la compilation complète du projet avant toute fusion.

## Équipe

Projet réalisé en équipe de six à Epitech :

- Alexandre Rousselle
- Orion Prieto
- Lohan Fernandes
- Maxens Lamotte
- Maksymilian Kusy
- Nathan Biliouw
