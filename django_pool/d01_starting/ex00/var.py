# Exercice 00 : mes premières variables
# Dossier : ex00/ - Fichier : var.py

def my_var():
    a = 42
    b = "42"
    c = "quarante-deux"
    d = 42.0
    e = True
    f = [42]
    g = {42: 42}
    h = (42,)
    i = set()

    print(f"{a} est de type {type(a)}")
    print(f"{b} est de type {type(b)}")
    print(f"{c} est de type {type(c)}")
    print(f"{d} est de type {type(d)}")
    print(f"{e} est de type {type(e)}")
    print(f"{f} est de type {type(f)}")
    print(f"{g} est de type {type(g)}")
    print(f"{h} est de type {type(h)}")
    print(f"{i} est de type {type(i)}")

if __name__ == '__main__':
    my_var()
