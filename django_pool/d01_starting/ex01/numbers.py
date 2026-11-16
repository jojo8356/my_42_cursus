# Exercice 01 : Nombres
# Dossier : ex01/ - Fichier : numbers.py

def read_numbers(filepath):
    """Lit les nombres d'un fichier séparés par une virgule."""
    with open(filepath, 'r') as f:
        content = f.read()
    numbers = content.strip().split(',')
    return [n.strip() for n in numbers]

def display_numbers(numbers):
    """Affiche les nombres un par ligne sans virgule."""
    for n in numbers:
        print(n)

def main():
    numbers = read_numbers("numbers.txt")
    display_numbers(numbers)

if __name__ == '__main__':
    main()
