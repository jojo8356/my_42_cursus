#!/usr/bin/env python3
"""Tests du jour d02 - Oob (POO Python).

Combine :
- le test officiel fourni avec le sujet (`d02.tar.gz` -> ex04/tests.py) ;
- les tests communautaires de `avallete/Python-Django-Pool` (d02/ex01..ex05) ;
- des tests maisons deduits du sujet pour ex00, ex05 et ex06.
"""

from __future__ import annotations

import shutil
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _harness import (  # noqa: E402
    POOL_DIR,
    Report,
    make_tmp,
    norm,
    run,
    run_py,
    stage,
)

DAY = POOL_DIR / "d02_oob"
TESTS = Path(__file__).resolve().parent
OFFICIAL = TESTS / "official" / "d02" / "ex04" / "tests.py"
COMMUNITY = TESTS / "community" / "d02"


def run_in_dir(report: Report, label: str, files: dict, script: str,
               args=(), must_contain=(), timeout=60) -> str:
    tmp = stage(files)
    rc, out, err = run_py(tmp / script, args, cwd=tmp, timeout=timeout)
    report.check(f"{label}: exit 0", rc == 0, err.strip())
    for needle in must_contain:
        report.check(f"{label}: contient {needle!r}", needle in out)
    return out


def main() -> int:
    r = Report("d02 - Oob")

    # --- ex00 : render.py ----------------------------------------------------
    example = make_tmp()
    shutil.copy(DAY / "ex00" / "render.py", example / "render.py")
    (example / "settings.py").write_text('name = "duoquadragintian"\n')
    (example / "file.template").write_text('<p>"-Who are you?\n-A {name}!"</p>\n')
    rc, out, err = run_py(example / "render.py", ["file.template"], cwd=example)
    r.check("ex00 render.py: exit 0", rc == 0, err)
    r.equals("ex00 render.py: exemple du sujet",
             (example / "file.html").read_text() if (example / "file.html").exists()
             else "<fichier absent>",
             '<p>"-Who are you?\n-A duoquadragintian!"</p>\n')
    rc, out, err = run_py(example / "render.py", ["file.txt"], cwd=example)
    r.check("ex00 render.py: mauvaise extension geree", rc == 0 and "Error" in out)
    rc, out, err = run_py(example / "render.py", ["nope.template"], cwd=example)
    r.check("ex00 render.py: fichier absent geree", rc == 0 and "Error" in out)
    rc, out, err = run_py(example / "render.py", [], cwd=example)
    r.check("ex00 render.py: mauvais nb d'arguments geree", rc == 0 and "Usage" in out)

    cv = stage({"render.py": DAY / "ex00" / "render.py",
                "myCV.template": DAY / "ex00" / "myCV.template",
                "settings.py": DAY / "ex00" / "settings.py"})
    rc, out, err = run_py(cv / "render.py", ["myCV.template"], cwd=cv)
    html = (cv / "myCV.html").read_text() if (cv / "myCV.html").exists() else ""
    for needle in ["<!DOCTYPE html>", "<html", "<head>", "<body>", "<title>",
                   "Dupont", "Jean", "30", "Developpeur"]:
        r.check(f"ex00 myCV.html: contient {needle!r}", needle in html)
    r.check("ex00 myCV.html: aucun motif subsistant", "{" not in html)

    # --- ex01 : intern.py (test communautaire) -------------------------------
    out = run_in_dir(r, "ex01 intern.py", {
        "intern.py": DAY / "ex01" / "intern.py",
        "test.py": COMMUNITY / "ex01" / "test.py",
    }, "test.py", must_contain=["Mark", "This is the worst coffee you ever tasted."])
    r.check("ex01 intern.py: nom par defaut",
            "My name? I'm nobody, an intern, I have no name." in out)

    # --- ex02 : beverages.py (test communautaire) ----------------------------
    out = run_in_dir(r, "ex02 beverages.py", {
        "beverages.py": DAY / "ex02" / "beverages.py",
        "test.py": COMMUNITY / "ex02" / "test.py",
    }, "test.py")
    r.check("ex02 beverages.py: prix a 2 decimales", "price : 0.30" in out)

    # --- ex03 : machine.py --------------------------------------------------
    # NB : le test communautaire d'ex03 (avallete) appelle serve() avec une
    # classe hors HotBeverage et sans gerer la panne : il est biaise, on lui
    # prefere un test deduit du sujet.
    tmp = stage({"machine.py": DAY / "ex03" / "machine.py",
                 "beverages.py": DAY / "ex03" / "beverages.py"})
    (tmp / "check.py").write_text('''
import random
from machine import CoffeeMachine
from beverages import HotBeverage, Coffee, Tea

EmptyCup = CoffeeMachine.EmptyCup
assert issubclass(EmptyCup, HotBeverage)
assert EmptyCup.name == "empty cup" and EmptyCup.price == 0.90
assert EmptyCup().description() == "An empty cup?! Gimme my money back!"
assert issubclass(CoffeeMachine.BrokenMachineException, Exception)
assert str(CoffeeMachine.BrokenMachineException()) == "This coffee machine has to be repaired."

random.seed(42)
cf = CoffeeMachine()
served = [type(cf.serve(Coffee)).__name__ for _ in range(10)]
assert set(served) <= {"Coffee", "EmptyCup"}, served
try:
    cf.serve(Coffee)
except CoffeeMachine.BrokenMachineException:
    print("breakdown_ok")
else:
    raise AssertionError("no breakdown after 10 drinks")
cf.repair()
print("after_repair:", type(cf.serve(Tea)).__name__)
print("ex03_ok")
''')
    rc, out, err = run_py(tmp / "check.py", cwd=tmp)
    r.check("ex03 machine.py: panne/repair/EmptyCup", rc == 0 and "ex03_ok" in out,
            out + err)

    # --- ex04 : elem.py (test officiel du sujet) ----------------------------
    tmp = stage({"elem.py": DAY / "ex04" / "elem.py", "tests.py": OFFICIAL})
    rc, out, err = run_py(tmp / "tests.py", cwd=tmp)
    r.check("ex04 elem.py: test officiel du sujet",
            rc == 0 and "Tests succeeded!" in out, out.strip() + err.strip())

    # --- ex05 : elements.py --------------------------------------------------
    # NB : le test communautaire d'ex05 (avallete) exige que Text n echappe
    # pas les guillemets, ce qui contredit le test officiel d'ex04 (sujet).
    tmp = stage({"elem.py": DAY / "ex05" / "elem.py",
                 "elements.py": DAY / "ex05" / "elements.py"})
    (tmp / "check.py").write_text('''
from elements import *
from elem import Text

print(repr(str(Html([Head(), Body()]))))
print(repr(str(Html())))
print(repr(str(Html(attr={"lang": "en"}))))
print(repr(str(Img(attr={"src": "http://i.imgur.com/pfp3T.jpg"}))))
print(repr(str(Html([Head(Title(Text("Hi"))), Body(H1(Text("Yo")))]))))
''')
    rc, out, err = run_py(tmp / "check.py", cwd=tmp)
    r.equals("ex05 elements.py: Html([Head(), Body()])", norm(out).splitlines()[0],
             repr("<html>\n  <head></head>\n  <body></body>\n</html>"))
    r.equals("ex05 elements.py: Html()", norm(out).splitlines()[1],
             repr("<html></html>"))
    r.equals("ex05 elements.py: attribut", norm(out).splitlines()[2],
             repr('<html lang="en"></html>'))
    r.equals("ex05 elements.py: img simple", norm(out).splitlines()[3],
             repr('<img src="http://i.imgur.com/pfp3T.jpg" />'))

    # --- ex06 : Page.py ------------------------------------------------------
    tmp = stage({"elem.py": DAY / "ex06" / "elem.py",
                 "elements.py": DAY / "ex06" / "elements.py",
                 "Page.py": DAY / "ex06" / "Page.py"})
    (tmp / "check.py").write_text('''
from Page import Page
from elements import *
from elem import Text

def show(label, page, expected):
    got = page.is_valid()
    print(f"{label}:{got}")
    assert got == expected, f"{label}: {got} != {expected}"

show("html_ok", Page(Html([Head(Title(Text("t"))), Body(H1(Text("h")))])), True)
show("html_bad_order", Page(Html([Body(), Head()])), False)
show("head_two_titles", Page(Head([Title(Text("a")), Title(Text("b"))])), False)
show("body_bad_child", Page(Body([Meta()])), False)
show("title_two_texts", Page(Title([Text("a"), Text("b")])), False)
show("p_with_elem", Page(P([Div()])), False)
show("span_ok", Page(Span([Text("a"), P(Text("b"))])), True)
show("ul_empty", Page(Ul()), False)
show("ul_ok", Page(Ul([Li(Text("a"))])), True)
show("tr_mixed", Page(Tr([Th(Text("a")), Td(Text("b"))])), False)
show("tr_ok", Page(Tr([Td(Text("a")), Td(Text("b"))])), True)
show("table_ok", Page(Table([Tr([Th(Text("a"))])])), True)
show("table_bad", Page(Table([H1(Text("a"))])), False)
show("div_ok", Page(Div([H2(Text("a")), Span(Text("b"))])), True)
print("str_doctype:", str(Page(Html([Head(Title(Text("t"))), Body()]))).startswith("<!DOCTYPE html>"))
print("str_no_doctype:", not str(Page(Div())).startswith("<!DOCTYPE"))
Page(Html([Head(Title(Text("t"))), Body()])).write_to_file("out.html")
print("file_ok:", open("out.html").read().startswith("<!DOCTYPE html>"))
''')
    rc, out, err = run_py(tmp / "check.py", cwd=tmp)
    r.check("ex06 Page.py: regles de validation du sujet",
            rc == 0 and "file_ok: True" in out, out + err)

    return r.summary()


if __name__ == "__main__":
    raise SystemExit(main())
