#!/usr/bin/env python3
"""Tests du jour d04 - Librairies.

ex02/ex03 sont testes avec un faux module `requests` (pas d'acces reseau
vers wikipedia depuis le bac a sable).  ex01/ex04/ex05 sont executes pour
de vrai.
"""

from __future__ import annotations

import os
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _harness import (  # noqa: E402
    POOL_DIR,
    Report,
    make_tmp,
    norm,
    read,
    run,
    run_py,
    stage,
)

DAY = POOL_DIR / "d04_lib"

ANTIGRAVITY_STUB = '''
import hashlib


def geohash(latitude, longitude, datedow):
    digest = hashlib.md5(datedow, usedforsecurity=False).hexdigest()
    fractions = []
    for half in (digest[:16], digest[16:]):
        value = float.fromhex('0.' + half)
        fractions.append(('%f' % value)[1:])
    print('%d%s %d%s' % (latitude, fractions[0], longitude, fractions[1]))
'''

FAKE_REQUESTS_WIKI = '''
class _Response:
    def __init__(self, payload, status=200):
        self._payload = payload
        self.status_code = status

    def json(self):
        return self._payload


def get(url, params=None, **kwargs):
    if params and params.get('srsearch'):
        return _Response({'query': {'search': [{'title': 'Chocolatine'}]}})
    return _Response({'query': {'pages': {'1': {'extract': 'Une chocolatine designe '
            'une viennoiserie au chocolat. [[category:Patisserie]] '
            "== References =="}}}})
'''

FAKE_REQUESTS_ROADS = '''
PAGES = {}


class RequestException(Exception):
    pass


class _Response:
    def __init__(self, text, status=200):
        self.text = text
        self.status_code = status

    def raise_for_status(self):
        if self.status_code >= 400:
            raise RequestException('status %d' % self.status_code)


def get(url, params=None, headers=None, **kwargs):
    name = url.rsplit('/', 1)[-1]
    return _Response(PAGES.get(name, '<html><body><h1>%s</h1>'
                              '<p>no link here</p></body></html>' % name))
'''


def main() -> int:
    r = Report("d04 - Librairies")

    # --- ex00 : geohashing.py -----------------------------------------------
    tmp = stage({"geohashing.py": DAY / "ex00" / "geohashing.py"})
    (tmp / "antigravity.py").write_text(ANTIGRAVITY_STUB)
    rc, out, err = run_py(tmp / "geohashing.py",
                          ["37.421542", "-122.085589", "2005-05-26-10458.68"],
                          cwd=tmp)
    r.check("ex00 geohashing.py: exit 0", rc == 0, err)
    r.check("ex00 geohashing.py: affiche un geohash",
            out.strip() != "" and " " in out.strip(), out)
    rc, out, err = run_py(tmp / "geohashing.py", [], cwd=tmp)
    r.check("ex00 geohashing.py: mauvais nb d'arguments", rc == 0 and "Usage" in out)
    rc, out, err = run_py(tmp / "geohashing.py", ["a", "b", "c"], cwd=tmp)
    r.check("ex00 geohashing.py: parametres invalides", rc == 0 and "Error" in out)

    # --- ex01 : my_script.sh + my_program.py --------------------------------
    tmp = stage({"my_script.sh": DAY / "ex01" / "my_script.sh",
                 "my_program.py": DAY / "ex01" / "my_program.py"})
    rc, out, err = run(["bash", "my_script.sh"], cwd=tmp, timeout=300)
    r.check("ex01 my_script.sh: exit 0", rc == 0, err.strip()[-400:])
    r.check("ex01 my_script.sh: affiche la version de pip", "pip" in out)
    r.check("ex01 my_script.sh: installe path.py dans local_lib",
            (tmp / "local_lib").exists())
    r.check("ex01 my_script.sh: log .log", (tmp / "install.log").exists())
    r.check("ex01 my_program.py: utilise path.py",
            "Hello from path.py!" in out)

    # --- ex02 : request_wikipedia.py (requests simule) ----------------------
    tmp = stage({"request_wikipedia.py": DAY / "ex02" / "request_wikipedia.py"})
    (tmp / "requests.py").write_text(FAKE_REQUESTS_WIKI)
    rc, out, err = run_py(tmp / "request_wikipedia.py", ["chocolatine"], cwd=tmp)
    wiki = tmp / "chocolatine.wiki"
    r.check("ex02 request_wikipedia.py: fichier .wiki cree", wiki.exists(), err)
    if wiki.exists():
        content = wiki.read_text()
        r.check("ex02 request_wikipedia.py: contenu net",
                "Une chocolatine designe" in content
                and "[[category" not in content and "==" not in content, content)
    rc, out, err = run_py(tmp / "request_wikipedia.py", [], cwd=tmp)
    r.check("ex02 request_wikipedia.py: sans argument", rc == 0 and "Usage" in out,
            out + err)

    # --- ex03 : roads_to_philosophy.py (requests simule) --------------------
    tmp = stage({"roads_to_philosophy.py": DAY / "ex03" / "roads_to_philosophy.py"})
    (tmp / "requests.py").write_text(
        FAKE_REQUESTS_ROADS +
        "\nPAGES['A'] = '<html><body><h1>A</h1><p><a href=\"/wiki/B\">B</a></p>"
        "</body></html>'\n"
        "PAGES['B'] = '<html><body><h1>B</h1><p><a href=\"/wiki/Philosophy\">P</a>"
        "</p></body></html>'\n"
        "PAGES['Philosophy'] = '<html><body><h1>Philosophy</h1><p>done</p>"
        "</body></html>'\n"
    )
    rc, out, err = run_py(tmp / "roads_to_philosophy.py", ["A"], cwd=tmp)
    r.check("ex03 roads_to_philosophy.py: route vers philosophie",
            rc == 0 and "3 roads from A to philosophy !" in out, out + err)
    (tmp / "requests.py").write_text(
        FAKE_REQUESTS_ROADS +
        "\nPAGES['Dead'] = '<html><body><h1>Dead</h1><p>nothing</p></body></html>'\n"
    )
    rc, out, err = run_py(tmp / "roads_to_philosophy.py", ["Dead"], cwd=tmp)
    r.check("ex03 roads_to_philosophy.py: impasse",
            rc == 0 and "It's a dead end !" in out, out + err)

    # --- ex04 : virtualenv --------------------------------------------------
    req = read(DAY / "ex04" / "requirement.txt")
    r.check("ex04 requirement.txt: django", "django" in req.lower())
    r.check("ex04 requirement.txt: psycopg2", "psycopg2" in req.lower())
    script = read(DAY / "ex04" / "my_script.sh")
    r.check("ex04 my_script.sh: cree django_venv", "django_venv" in script)
    r.check("ex04 my_script.sh: installe requirement.txt", "requirement.txt" in script)

    # --- ex05 : hello world -------------------------------------------------
    project = DAY / "ex05"
    tmp = make_tmp()
    (tmp / "probe.py").write_text(
        "import os, sys, django\n"
        f"sys.path.insert(0, {str(project)!r})\n"
        "os.environ['DJANGO_SETTINGS_MODULE'] = 'hello_project.settings'\n"
        "django.setup()\n"
        "from django.test import Client\n"
        "c = Client()\n"
        "for url in ('/helloworld', '/helloworld/'):\n"
        "    r = c.get(url)\n"
        "    print(url, r.status_code, r.get('Content-Type', ''))\n"
        "    if r.status_code == 200:\n"
        "        print(r.content.decode())\n"
    )
    rc, out, err = run_py(tmp / "probe.py", timeout=120)
    r.check("ex05 helloworld: repond 200", "200" in out, out + err)
    r.check("ex05 helloworld: affiche 'Hello World !'", "Hello World !" in out)

    return r.summary()


if __name__ == "__main__":
    raise SystemExit(main())
