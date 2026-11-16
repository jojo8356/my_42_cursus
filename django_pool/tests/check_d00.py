#!/usr/bin/env python3
"""Tests du jour d00 - Initiation (HTML, CSS, JavaScript).

Les ressources de reference (popup.js, style.css, page.png, file*.js,
sources ex05) viennent de la tarball officielle `d00.tar.gz`.
"""

from __future__ import annotations

import hashlib
import http.server
import re
import socketserver
import sys
import threading
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _harness import FIXTURES_DIR, POOL_DIR, Report, read, run  # noqa: E402

DAY = POOL_DIR / "d00_initiation"
FIX = FIXTURES_DIR / "d00"


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def start_redirect_server(target: str):
    """Serveur local qui reecrit toute requete vers `target`."""

    class Handler(http.server.BaseHTTPRequestHandler):
        def do_HEAD(self):
            self.send_response(302)
            self.send_header("Location", target)
            self.end_headers()

        def do_GET(self):
            self.do_HEAD()

        def log_message(self, *args):
            pass

    server = socketserver.TCPServer(("127.0.0.1", 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    return server, server.server_address[1]


def main() -> int:
    r = Report("d00 - Initiation")

    # --- ex00 : myawesomescript.sh ------------------------------------------
    script = DAY / "ex00" / "myawesomescript.sh"
    r.check("ex00 myawesomescript.sh: executable", script.exists()
            and script.stat().st_mode & 0o111, "bit d'execution manquant")
    body = read(script)
    for cmd in ("curl", "grep", "cut"):
        r.check(f"ex00 myawesomescript.sh: utilise {cmd}", cmd in body)
    server, port = start_redirect_server("http://example.com/")
    try:
        rc, out, err = run([str(script), f"http://127.0.0.1:{port}/bit.ly/x"],
                           timeout=30)
        r.equals("ex00 myawesomescript.sh: resolve la redirection",
                 out.strip(), "http://example.com/")
    finally:
        server.shutdown()

    # --- ex01 : cv.html ------------------------------------------------------
    cv = read(DAY / "ex01" / "cv.html")
    head = cv.split("<head>", 1)[1].split("</head>", 1)[0] if "<head>" in cv else ""
    r.check("ex01 cv.html: balise <title>", "<title>" in cv)
    r.check("ex01 cv.html: balise <h1>", "<h1>" in cv)
    for tag in ("<table", "<th", "<tr", "<td"):
        r.check(f"ex01 cv.html: balise {tag}", tag in cv)
    r.check("ex01 cv.html: liste <ul>/<li>", "<ul>" in cv and "<li>" in cv)
    r.check("ex01 cv.html: liste <ol>/<li>", "<ol>" in cv and "<li>" in cv)
    r.check("ex01 cv.html: fond/forme separes (style dans le head)",
            "border-collapse" in head and "solid" in head)
    r.check("ex01 cv.html: #424242 en attribut style",
            re.search(r'style="[^"]*#424242', cv) is not None)

    # --- ex02 : form.html + popup.js ----------------------------------------
    form = read(DAY / "ex02" / "form.html")
    r.check("ex02 popup.js: present et inchange",
            (DAY / "ex02" / "popup.js").exists()
            and sha(DAY / "ex02" / "popup.js") == sha(FIX / "ex02" / "popup.js"))
    r.check("ex02 form.html: importe popup.js", 'src="popup.js"' in form)
    r.check("ex02 form.html: onclick displayFormContents",
            "displayFormContents();" in form)
    for field in ("firstname", "name", "age", "phone", "email", "student"):
        r.check(f"ex02 form.html: champ {field}", f'id="{field}"' in form)
    for field in ("text", "number", "tel", "email", "checkbox", "radio"):
        r.check(f"ex02 form.html: type {field}", f'type="{field}"' in form)
    for value in ("Male", "Female", "Other"):
        r.check(f"ex02 form.html: radio {value}", f'value="{value}"' in form)
    # popup.js lit les ids gender_male/female/other
    for gid in ("gender_male", "gender_female", "gender_other"):
        r.check(f"ex02 form.html: id {gid} (requis par popup.js)",
                f'id="{gid}"' in form)

    # --- ex03 : copy.html + style.css ---------------------------------------
    css = DAY / "ex03" / "style.css"
    r.check("ex03 style.css: fourni et inchange",
            css.exists() and sha(css) == sha(FIX / "ex03" / "style.css"))
    copy = read(DAY / "ex03" / "copy.html")
    r.check("ex03 copy.html: utilise style.css", 'href="style.css"' in copy)
    r.check("ex03 copy.html: structure html complete",
            "<!DOCTYPE html>" in copy and "<html" in copy
            and "<head>" in copy and "<body>" in copy)

    # --- ex04 : snippets.html -----------------------------------------------
    snippets = read(DAY / "ex04" / "snippets.html")
    for i in (1, 2, 3, 4):
        r.check(f"ex04 snippets.html: importe file{i}.js",
                f'src="file{i}.js"' in snippets)
        r.check(f"ex04 file{i}.js: présent",
                (DAY / "ex04" / f"file{i}.js").exists())
    # pas de javascript inline : uniquement des balises script src
    inline = re.findall(r"<script(?![^>]*src=)[^>]*>", snippets)
    r.check("ex04 snippets.html: aucun javascript inline", not inline, str(inline))

    # --- ex05 : index.html corrige ------------------------------------------
    index = DAY / "ex05" / "index.html"
    html = read(index)
    original = read(FIX / "ex05" / "index.html")
    for needle in ["Art Gallery Blog", "Opening Date Announced",
                   "Anticipation Mounting", "Will the Art Gallery open this year?",
                   "Artist Profile: Armando Blontio", "Camry Plaza",
                   "September 16, 2013", "armando-blontio.jpg", "camry-plaza.jpg"]:
        r.check(f"ex05 index.html: contenu preserve ({needle})", needle in html)
    # le contenu source doit etre present dans son integralite (textes)
    for text in ["Covering all the Art Galleries across the globe",
                 "invitation-only", "misrespresented", "Read More"]:
        r.check(f"ex05 index.html: texte source present ({text})", text in html)
    for bad in ["</script\n", "</meta>", "<asside>", "</>", "&&raquo;", "<times>",
                'href=>']:
        r.check(f"ex05 index.html: plus d'erreur {bad!r}", bad not in html)
    r.check("ex05 index.html: doctype presente", html.lstrip().startswith("<!DOCTYPE html>"))
    try:
        import html5lib
        parser = html5lib.HTMLParser(strict=True)
        parser.parse(html)
        r.ok("ex05 index.html: analyse html5 stricte sans erreur")
    except ImportError:
        r.skip("ex05 index.html: html5lib absent", "pip install html5lib")
    except Exception as exc:  # noqa: BLE001
        r.fail("ex05 index.html: analyse html5 stricte sans erreur", str(exc))

    return r.summary()


if __name__ == "__main__":
    raise SystemExit(main())
