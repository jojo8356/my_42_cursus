# Exercice 00 : mes premières variables
# Dossier : ex00/ - Fichier : var.py

def my_var():
    a = 42
    b = 42
    c = "quarante-deux"
    d = 42.0
    e = True
    f = [42]
    g = {42: 42}
    h = (42,)
    i = set()

    print(f"{a} est de type {type(a).__name__}")
    print(f"{b} est de type {type(b).__name__}")
    print(f"{c} est de type {type(c).__name__}")
    print(f"{d} est de type {type(d).__name__}")
    print(f"{e} est de type {type(e).__name__}")
    print(f"{f} est de type {type(f).__name__}")
    print(f"{g} est de type {type(g).__name__}")
    print(f"{h} est de type {type(h).__name__}")
    print(f"{i} est de type {type(i).__name__}")

if __name__ == '__main__':
    my_var()
