# FT_CONTAINERS — Énoncé (résumé officiel, 42 School)

## Descriptif
Recoder les conteneurs de la bibliothèque standard C++ **en C++98**, dans le namespace **`ft`**,
en respectant les mêmes comportements et interfaces que les conteneurs STL originaux.

## Partie obligatoire
Implémenter les conteneurs suivants et remettre les fichiers `<container>.hpp` correspondants :
- **vector** (allocé dynamiquement ; la spécialisation `vector<bool>` n'est pas requise)
- **map** (arbre rouge-noir ou AVL)
- **stack** (utilise la classe `vector` de l'étudiant comme conteneur sous-jacent par défaut ;
  doit rester compatible avec d'autres conteneurs, y compris ceux de la STL)

On doit également implémenter :
- **iterator_traits**
- **reverse_iterator**
- **enable_if**
- **is_integral**
- **equal** et/ou **lexicographical_compare**
- **std::pair** / **make_pair**

## Partie bonus
Un conteneur supplémentaire :
- **set** (un arbre **rouge-noir** est obligatoire)

Bonus complémentaires (implémentés dans cette version pour une couverture complète) :
- **deque**, **queue**, **multimap**, **multiset**

Le bonus n'est évalué que si la partie obligatoire est **PARFAITE**.

## Règles
- Le namespace doit être **`ft`**.
- Chaque structure de données interne doit être **logique et justifiée** (un simple tableau pour
  `map` n'est pas acceptable → arbre rouge-noir pour map/set/multimap/multiset).
- **`std::allocator`** doit être utilisé.
- Ne pas implémenter plus de fonctions publiques que les conteneurs standards (le reste est
  `private`/`protected`). Chaque fonction/variable publique doit être justifiée.
- **Toutes** les fonctions membres, fonctions non-membres et surcharges des conteneurs standards
  sont attendues.
- Si un conteneur possède un système d'itérateurs, il **doit** être implémenté.
- Code **C++98** strict (aucune fonctionnalité C++11).
