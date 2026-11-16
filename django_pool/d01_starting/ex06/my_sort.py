# Exercice 06 : Tri d'un dictionnaire
# Dossier : ex06/ - Fichier : my_sort.py

def sort_musicians():
    d = {
        'Hendrix': '1942',
        'Allman': '1946',
        'King': '1925',
        'Clapton': '1945',
        'Johnson': '1911',
        'Berry': '1926',
        'Vaughan': '1954',
        'Cooder': '1947',
        'Page': '1944',
        'Richards': '1943',
        'Hammett': '1962',
        'Cobain': '1967',
        'Garcia': '1942',
        'Beck': '1944',
        'Santana': '1947',
        'Ramone': '1948',
        'White': '1975',
        'Frusciante': '1970',
        'Thompson': '1949',
        'Burton': '1939',
    }

    # Trier par année croissante, puis par nom alphabétique
    sorted_items = sorted(d.items(), key=lambda x: (x[1], x[0]))

    for name, year in sorted_items:
        print(name)

if __name__ == '__main__':
    sort_musicians()
