<div align="center">

# CPP Modules 00–09

**42 School — C++ Common Core (cpp_00 → cpp_09) : 37 exercices, 100 % implémentés avec les bonus**

[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B98-00599C?style=flat-square&logo=cpp&logoColor=white)](https://en.cppreference.com/w/)
[![Compiler Flags](https://img.shields.io/badge/Flags--Wall_-Wextra_-Werror_-std%3Dc%2B%2B98-success?style=flat-square)](#compilation)
[![Tests](https://img.shields.io/badge/Tests-93%2F93-brightgreen?style=flat-square)](#tests--validation)
[![Sanitizers](https://img.shields.io/badge/Sanitizers-ASan%20%2B%20TSan-brightgreen?style=flat-square)](#tests--validation)

**[ 🇬🇧 English ](#english) · [ 🇫🇷 Français ](#français)**

</div>

---

## 🇬🇧 English

### Overview

Implementation of the complete **C++ Modules** curriculum (modules 00–09) from École 42, written in **strict C++98**. All **37 exercises** are implemented with their **bonus** features and validated by an automated test suite of **93 checks**, including AddressSanitizer and ThreadSanitizer builds.

### Projects Matrix

| Module | Exercises | Key Concepts | Bonus |
| :--- | :--- | :--- | :---: |
| **cpp_00** | Megaphone, PhoneBook, Account | I/O, string handling, interactive input | ✅ Account |
| **cpp_01** | Zombie, zombieHorde, brain, Weapon/HumanA/HumanB, sed, Harl, Harl filter | Inheritance, references vs pointers, `std::sed` | ✅ All |
| **cpp_02** | Fixed (canonical), int/float, operators, BSP | Orthodox canonical form, bit-level fixed-point | ✅ BSP |
| **cpp_03** | ClapTrap, ScavTrap, FragTrap, DiamondTrap | Virtual inheritance, multiple inheritance | ✅ DiamondTrap |
| **cpp_04** | Animal/Dog/Cat/WrongAnimal/WrongCat, Brain, abstract, AMateria | Polymorphism, deep copy, interfaces | ✅ All |
| **cpp_05** | Bureaucrat, Form, AForm + Shrubbery/Robotomy/Presidential, Intern | Exceptions, grades, form execution | ✅ All forms |
| **cpp_06** | ScalarConverter, Serializer/Data, Base/A/B/C | `strtod`/`strtol`, `uintptr_t`, `dynamic_cast` | ✅ All |
| **cpp_07** | templates swap/min/max, iter | Function templates, const/non-const overloads | ✅ All |
| **cpp_08** | easyfind, Span, MutantStack | STL containers & algorithms (mandatory) | ✅ All |
| **cpp_09** | BitcoinExchange, RPN, PmergeMe | STL (`map`/`deque`/`vector`+`list`), Ford-Johnson | ✅ All |

### Compilation

Each exercise has its own `Makefile` with the standard targets `all`, `clean`, `fclean`, `re`:

```bash
# Build a single exercise
make -C cpp_modules/cpp_09/ex02

# Rebuild everything in a module (run from the module dir)
cd cpp_modules/cpp_09 && for ex in ex00 ex01 ex02; do make -C $ex re; done
```

All code compiles cleanly under `c++ -Wall -Wextra -Werror -std=c++98`.

### Tests & Validation

```bash
python3 cpp_modules/tests/test_cpp_modules.py
```

The suite runs **93 checks**: 37 isolated Makefile builds, 50 functional scenarios and 6 sanitizer
build/runs. It covers:
- All **37 exercise builds** under `-Wall -Wextra -Werror -std=c++98`; builds happen in temporary
  copies and leave the source tree untouched
- **Functional behavior and expected output** across all modules
- **Edge cases** (empty input, negative values, non-digits, `inf`/`nan`) and CLI argument validation
- **PmergeMe stress tests** on 3,000 distinct even and 3,000 distinct odd integers
- **ASan and TSan** instrumented builds/runs on selected memory- and inheritance-heavy exercises;
  sanitizer checks are skipped when that sanitizer is unavailable

### Rules Compliance

- **C++98 strict**, `-Wall -Wextra -Werror`
- `printf` / `*alloc()` / `free()` forbidden
- STL allowed **only** in modules 08–09
- `using namespace` and `friend` forbidden
- Orthodox canonical form mandatory **cpp_02 → cpp_09**
- Include guards on every `.hpp`; implementation in `.hpp` = 0 (templates exempt)

### Subjects

Official subject texts are in [`subjects/`](subjects/) (`CPP_Module_00.txt` … `CPP_Module_09.pdf`).

---

## 🇫🇷 Français

### Présentation

Implémentation complète des **modules CPP 00–09** de l'École 42, en **C++98 strict**. Les **37 exercices** sont tous implémentés avec leurs **bonus** et validés par une suite de tests automatisés de **93 checks**, incluant des builds AddressSanitizer et ThreadSanitizer.

### Catalogue

| Module | Exercices | Concepts clés | Bonus |
| :--- | :--- | :--- | :---: |
| **cpp_00** | Megaphone, PhoneBook, Account | E/S, manipulation de chaînes, entrée interactive | ✅ Account |
| **cpp_01** | Zombie, zombieHorde, brain, Weapon/HumanA/HumanB, sed, Harl, Harl filter | Héritage, références vs pointeurs, `std::sed` | ✅ Tout |
| **cpp_02** | Fixed (canonique), int/float, opérateurs, BSP | Forme canonique, nombres à virgule fixe | ✅ BSP |
| **cpp_03** | ClapTrap, ScavTrap, FragTrap, DiamondTrap | Héritage virtuel, héritage multiple | ✅ DiamondTrap |
| **cpp_04** | Animal/Dog/Cat/WrongAnimal/WrongCat, Brain, abstrait, AMateria | Polymorphisme, copie profonde, interfaces | ✅ Tout |
| **cpp_05** | Bureaucrat, Form, AForm + Shrubbery/Robotomy/Presidential, Intern | Exceptions, grades, exécution de forms | ✅ Toutes les forms |
| **cpp_06** | ScalarConverter, Serializer/Data, Base/A/B/C | `strtod`/`strtol`, `uintptr_t`, `dynamic_cast` | ✅ Tout |
| **cpp_07** | templates swap/min/max, iter | Templates de fonctions, surcharges const/non-const | ✅ Tout |
| **cpp_08** | easyfind, Span, MutantStack | Containers & algorithmes STL (obligatoire) | ✅ Tout |
| **cpp_09** | BitcoinExchange, RPN, PmergeMe | STL (`map`/`deque`/`vector`+`list`), Ford-Johnson | ✅ Tout |

### Compilation

Chaque exercice dispose de son propre `Makefile` avec les cibles `all`, `clean`, `fclean`, `re` :

```bash
# Compiler un exercice
make -C cpp_modules/cpp_09/ex02
```

Tout le code compile proprement sous `c++ -Wall -Wextra -Werror -std=c++98`.

### Tests & Validation

```bash
python3 cpp_modules/tests/test_cpp_modules.py
```

La suite exécute **93 checks** : 37 builds isolés via les Makefiles, 50 scénarios fonctionnels et
6 builds/exécutions sous sanitizers. Elle couvre :
- Les **37 builds** sous `-Wall -Wextra -Werror -std=c++98`, dans des copies temporaires qui ne
  modifient pas les sources
- Le **comportement fonctionnel et les sorties attendues** dans tous les modules
- Les **cas limites** (entrée vide, valeurs négatives, non-chiffres, `inf`/`nan`) et la validation
  des arguments CLI
- **PmergeMe** sur 3 000 entiers pairs distincts et 3 000 entiers impairs distincts
- Des exécutions instrumentées **ASan et TSan** sur des exercices sélectionnés ; les checks d’un
  sanitizer sont ignorés si celui-ci n’est pas disponible

### Respect des règles

- **C++98 strict**, `-Wall -Wextra -Werror`
- `printf` / `*alloc()` / `free()` interdits
- STL autorisée **uniquement** dans les modules 08–09
- `using namespace` et `friend` interdits
- Forme canonique obligatoire **cpp_02 → cpp_09**
- Guards d'inclusion sur chaque `.hpp` ; implémentation en `.hpp` = 0 (templates exclus)

### Énoncés

Les énoncés officiels se trouvent dans [`subjects/`](subjects/) (`CPP_Module_00.txt` … `CPP_Module_09.pdf`).
