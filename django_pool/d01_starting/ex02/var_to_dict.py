# Exercice 02 : Mon premier dictionnaire
# Dossier : ex02/ - Fichier : var_to_dict.py

def transform_and_display():
    d = [('Hendrix', '1942'), ('Allman', '1946'), ('King', '1925'),
         ('Clapton', '1945'), ('Johnson', '1911'), ('Berry', '1926'),
         ('Vaughan', '1954'), ('Cooler', '1947'), ('Page', '1944'),
         ('Richards', '1943'), ('Hammett', '1962'), ('Cobain', '1967'),
         ('Garcia', '1942'), ('Beck', '1944'), ('Santana', '1947'),
         ('Ramone', '1948'), ('White', '1975'), ('Frusciante', '1970'),
         ('Thompson', '1949'), ('Burton', '1939')]

    result = {}
    for name, year in d:
        result[year] = name

    for year in sorted(result.keys()):
        print(f"{year} : {result[year]}")

if __name__ == '__main__':
    transform_and_display()
