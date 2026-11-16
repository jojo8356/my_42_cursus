# Exercice 03 : Recherche par clé
# Dossier : ex03/ - Fichier : capital_city.py
import sys

def find_capital(state):
    """Retourne la capitale d'un état donné."""
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

    if state in states:
        code = states[state]
        return capital_cities.get(code, "Unknown state")
    return "Unknown state"

def main():
    if len(sys.argv) != 2:
        return
    state = sys.argv[1]
    result = find_capital(state)
    print(result)

if __name__ == '__main__':
    main()
