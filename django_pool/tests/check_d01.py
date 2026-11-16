#!/usr/bin/env python3
"""Tests du jour d01 - Starting (Python de base).

Les sorties attendues sont celles du sujet officiel
(`Pool/Django Pool/Django-0-Starting`, t.e. `fr.subject.pdf`).
"""

from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _harness import (  # noqa: E402
    FIXTURES_DIR,
    POOL_DIR,
    Report,
    norm,
    run,
    run_py,
    stage,
)

DAY = POOL_DIR / "d01_starting"
FIX = FIXTURES_DIR / "d01"

EXPECTED_VAR = """42 est de type <class 'int'>
42 est de type <class 'str'>
quarante-deux est de type <class 'str'>
42.0 est de type <class 'float'>
True est de type <class 'bool'>
[42] est de type <class 'list'>
{42: 42} est de type <class 'dict'>
(42,) est de type <class 'tuple'>
set() est de type <class 'set'>"""

# nom -> annee, recopie du sujet
MUSICIANS = [
    ("Hendrix", "1942"), ("Allman", "1946"), ("King", "1925"),
    ("Clapton", "1945"), ("Johnson", "1911"), ("Berry", "1926"),
    ("Vaughan", "1954"), ("Cooder", "1947"), ("Page", "1944"),
    ("Richards", "1943"), ("Hammett", "1962"), ("Cobain", "1967"),
    ("Garcia", "1942"), ("Beck", "1944"), ("Santana", "1947"),
    ("Ramone", "1948"), ("White", "1975"), ("Frusciante", "1970"),
    ("Thompson", "1949"), ("Burton", "1939"),
]


def expected_by_year() -> dict[str, str]:
    out: dict[str, list[str]] = {}
    for name, year in MUSICIANS:
        out.setdefault(year, []).append(name)
    return {y: " ".join(names) for y, names in out.items()}


def expected_sorted() -> list[str]:
    return [name for name, _ in sorted(MUSICIANS, key=lambda x: (x[1], x[0]))]


def main() -> int:
    r = Report("d01 - Starting")

    # --- ex00 : var.py -------------------------------------------------------
    rc, out, err = run_py(DAY / "ex00" / "var.py")
    r.check("ex00 var.py: exit 0", rc == 0, err)
    r.equals("ex00 var.py: sortie exacte", norm(out), EXPECTED_VAR)

    # --- ex01 : numbers.py ---------------------------------------------------
    tmp = stage({"numbers.py": DAY / "ex01" / "numbers.py",
                 "numbers.txt": FIX / "ex01" / "numbers.txt"})
    rc, out, err = run_py(tmp / "numbers.py", cwd=tmp)
    r.check("ex01 numbers.py: exit 0", rc == 0, err)
    r.equals("ex01 numbers.py: 1..100 un par ligne",
             norm(out), "\n".join(str(i) for i in range(1, 101)))

    # --- ex02 : var_to_dict.py ----------------------------------------------
    rc, out, err = run_py(DAY / "ex02" / "var_to_dict.py")
    r.check("ex02 var_to_dict.py: exit 0", rc == 0, err)
    got: dict[str, str] = {}
    for line in norm(out).splitlines():
        if " : " in line:
            year, name = line.split(" : ", 1)
            got[year.strip()] = name.strip()
    want = expected_by_year()
    missing = {y: n for y, n in want.items() if got.get(y) != n}
    r.check("ex02 var_to_dict.py: annee -> musiciens exacts",
            not missing, f"manquants/differents: {missing}")

    # --- ex03 : capital_city.py ---------------------------------------------
    cases = [
        (["Oregon"], "Salem"),
        (["Ile-De-France"], "Unknown state"),
        ([], ""),
        (["Oregon", "Alabama"], ""),
        (["Oregon", "Alabama", "Ile-De-France"], ""),
    ]
    for args, want_out in cases:
        rc, out, err = run_py(DAY / "ex03" / "capital_city.py", args)
        r.equals(f"ex03 capital_city.py {args or '(aucun arg)'}",
                 norm(out), want_out)

    # --- ex04 : state.py -----------------------------------------------------
    for args, want_out in [(["Salem"], "Oregon"), (["Paris"], "Unknown capital city"),
                           ([], ""), (["Salem", "Denver"], "")]:
        rc, out, err = run_py(DAY / "ex04" / "state.py", args)
        r.equals(f"ex04 state.py {args or '(aucun arg)'}", norm(out), want_out)

    # --- ex05 : all_in.py ----------------------------------------------------
    example = "New jersey, Tren ton, NewJersey, Trenton, toto, , sAlem"
    want_out = "\n".join([
        "Trenton is the capital of New Jersey",
        "Tren ton is neither a capital city nor a state",
        "NewJersey is neither a capital city nor a state",
        "Trenton is the capital of New Jersey",
        "toto is neither a capital city nor a state",
        "Salem is the capital of Oregon",
    ])
    rc, out, err = run_py(DAY / "ex05" / "all_in.py", [example])
    r.equals("ex05 all_in.py: exemple du sujet", norm(out), want_out)
    rc, out, err = run_py(DAY / "ex05" / "all_in.py", [])
    r.equals("ex05 all_in.py: sans argument", norm(out), "")
    rc, out, err = run_py(DAY / "ex05" / "all_in.py", ["a,,b"])
    r.equals("ex05 all_in.py: double virgule", norm(out), "")
    rc, out, err = run_py(DAY / "ex05" / "all_in.py", ["colorado"])
    r.equals("ex05 all_in.py: casse espaces", norm(out),
             "Denver is the capital of Colorado")

    # --- ex06 : my_sort.py ---------------------------------------------------
    rc, out, err = run_py(DAY / "ex06" / "my_sort.py")
    r.equals("ex06 my_sort.py: tri annee puis nom",
             norm(out), "\n".join(expected_sorted()))

    # --- ex07 : periodic_table.py -------------------------------------------
    tmp = stage({"periodic_table.py": DAY / "ex07" / "periodic_table.py",
                 "periodic_table.txt": FIX / "ex07" / "periodic_table.txt"})
    rc, out, err = run_py(tmp / "periodic_table.py",
                          [FIX / "ex07" / "periodic_table.txt"], cwd=tmp)
    html_path = tmp / "periodic_table.html"
    if not r.check("ex07 periodic_table.py: fichier html genere",
                   html_path.exists(), err):
        return r.summary()
    html = html_path.read_text(encoding="utf-8", errors="replace")
    r.check("ex07 periodic_table.py: 88 elements rendus",
            html.count("<h4>") == 88, f"{html.count('<h4>')} <h4>")
    for needle in ["<table>", "<h4>Hydrogen</h4>", "<li>No 1</li>",
                   "<li>H</li>", "1.00794"]:
        r.check(f"ex07 periodic_table.py: contient {needle!r}", needle in html)
    # le layout Mendeleiev demande 7 periodes x 18 colonnes, cases vides incluses
    rows = html.split("<tr>")[1:]
    r.equals("ex07 periodic_table.py: 7 periodes", len(rows), 7)
    if rows:
        r.equals("ex07 periodic_table.py: 18 colonnes par periode",
                 rows[0].count("<td"), 18)
        r.check("ex07 periodic_table.py: cases vides la ou il en faut",
                "<td></td>" in rows[0], "aucune case vide dans la premiere periode")

    return r.summary()


if __name__ == "__main__":
    raise SystemExit(main())
