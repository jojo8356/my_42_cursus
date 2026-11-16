# Exercice 04 : Recherche par valeur
# Dossier : ex04/ - Fichier : state.py
import sys

def find_state(capital):
    """Retourne l'état correspondant à une capitale donnée."""
    states = {
        "Oregon": "OR",
        "Alabama": "AL",
        "New Jersey": "NJ",
        "Colorado": "CO"
    }
    capital_cities = {
        "OR": "Salem",
        "AL": "Montgomery",
        "NJ": "Trenton",
        "CO": "Denver"
    }

    # Inverser le dictionnaire pour recherche par valeur
    reverse = {v: k for k, v in capital_cities.items()}
    if capital in reverse:
        return reverse[capital]
    return "Unknown capital city"

def main():
    if len(sys.argv) != 2:
        return
    capital = sys.argv[1]
    result = find_state(capital)
    print(result)

if __name__ == '__main__':
    main()
