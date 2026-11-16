# Exercice 05 : Recherche par clé ou par valeur
# Dossier : ex05/ - Fichier : all_in.py
import sys

def search(expressions):
    """Recherche chaque expression dans les dictionnaires."""
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

    # Pour la recherche insensible à la casse
    states_lower = {k.lower(): k for k in states.keys()}
    capitals_lower = {k.lower(): k for k in capital_cities.values()}

    results = []
    for expr in expressions:
        expr_display = expr.strip()
        expr_clean = expr_display.lower()
        if not expr_clean:
            continue
        # Chercher si c'est un état
        if expr_clean in states_lower:
            state = states_lower[expr_clean]
            code = states[state]
            capital = capital_cities[code]
            results.append(f"{capital} is the capital of {state}")
        # Chercher si c'est une capitale
        elif expr_clean in capitals_lower:
            capital = capitals_lower[expr_clean]
            # Trouver l'état correspondant
            state = None
            for s, c in states.items():
                if capital_cities[c] == capital:
                    state = s
                    break
            if state:
                results.append(f"{capital} is the capital of {state}")
            else:
                results.append(f"{expr_display} is neither a capital city nor a state")
        else:
            results.append(f"{expr_display} is neither a capital city nor a state")
    return results

def main():
    if len(sys.argv) != 2:
        return
    args = sys.argv[1]
    # Détecter les doubles virgules
    if ",," in args:
        return
    expressions = args.split(',')
    results = search(expressions)
    for r in results:
        print(r)

if __name__ == '__main__':
    main()
