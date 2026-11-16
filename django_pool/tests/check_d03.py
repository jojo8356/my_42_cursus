#!/usr/bin/env python3
"""Tests du jour d03 - Base Django (projet d04)."""

from __future__ import annotations

import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _harness import POOL_DIR, Report, make_tmp, run_py  # noqa: E402

PROJECT = POOL_DIR / "d03_base_django" / "d04"

PROBE = r'''
import os
import re
import sys
import django

sys.path.insert(0, sys.argv[1])
os.environ['DJANGO_SETTINGS_MODULE'] = 'd04.settings'
django.setup()

from django.conf import settings
from django.core.management import call_command
from django.test import Client

settings.LOG_FILE_PATH = os.path.join(sys.argv[2], 'logs.txt')
call_command('migrate', run_syncdb=True, verbosity=0, interactive=False)

c = Client()
out = []


def show(label, url, **kwargs):
    r = c.get(url, follow=True, **kwargs)
    out.append(f"{label}|{r.status_code}|{r.get('Content-Type', '').split(';')[0]}")
    html = r.content.decode('utf-8', 'replace')
    return r, html


r, html = show('ex00', '/ex00')
out.append('ex00_title|' + ('Ex00 : Markdown Cheatsheet.' in html and 'yes' or 'no'))
show('ex00_slash', '/ex00/')
show('ex00_404', '/ex00foo')

for label, url, title in [
    ('ex01_django', '/ex01/django', 'Ex01 : Django, framework web.'),
    ('ex01_django_slash', '/ex01/django/', 'Ex01 : Django, framework web.'),
    ('ex01_affichage', '/ex01/affichage', "Ex01 : Processus d'affichage d'une page statique."),
    ('ex01_templates', '/ex01/templates', 'Ex01 : Moteur de template.'),
]:
    r, html = show(label, url)
    out.append(f"{label}_title|{title in html and 'yes' or 'no'}")

_, html = show('ex01_django_style', '/ex01/django')
out.append('style1_used|' + ('style1.css' in html and 'yes' or 'no'))
_, html = show('ex01_templates_style', '/ex01/templates')
out.append('style2_used|' + ('style2.css' in html and 'yes' or 'no'))
out.append('style1_absent_on_templates|' + ('style1.css' not in html and 'yes' or 'no'))

r, html = show('ex02', '/ex02')
out.append('ex02_form|' + ('<form' in html and 'yes' or 'no'))
r = c.post('/ex02/', {'text_input': 'hello world'})
out.append('ex02_post|' + str(r.status_code))
html = r.content.decode('utf-8', 'replace')
out.append('ex02_history|' + ('hello world' in html and 'yes' or 'no'))
out.append('ex02_logfile|' + (os.path.exists(settings.LOG_FILE_PATH) and 'yes' or 'no'))
if os.path.exists(settings.LOG_FILE_PATH):
    out.append('ex02_logcontent|' + ('hello world' in open(settings.LOG_FILE_PATH).read() and 'yes' or 'no'))

r, html = show('ex03', '/ex03')
out.append('ex03_th|' + str(html.count('<th')) + '|' + str(html.count('</th>')))
out.append('ex03_tr|' + str(html.count('<tr>')) + '|' + str(html.count('</tr>')))
out.append('ex03_td|' + str(html.count('<td')) + '|' + str(html.count('</td>')))
shades = re.findall(r'background-color: (#[0-9a-fA-F]{6});', html)
out.append('ex03_shades|' + str(len(shades)) + '|' + str(len(set(shades))))

print('##')
print('\n'.join(out))
'''


def main() -> int:
    r = Report("d03 - Base Django")
    tmp = make_tmp()
    probe = tmp / "probe.py"
    probe.write_text(PROBE)
    rc, out, err = run_py(probe, [str(PROJECT), str(tmp)], timeout=180)
    if rc != 0:
        r.fail("d04: probe Django", err[-800:])
        return r.summary()
    body = out.split("##\n", 1)[1] if "##\n" in out else ""
    results = {}
    for line in body.strip().splitlines():
        if "|" in line:
            key, _, value = line.partition("|")
            results[key] = value

    def val(key):
        return results.get(key, "<absent>")

    r.equals("ex00: /ex00 repond 200", val("ex00"), "200|text/html")
    r.equals("ex00: titre", val("ex00_title"), "yes")
    r.equals("ex00: /ex00/ fonctionne", val("ex00_slash").split("|")[0], "200")
    r.equals("ex00: /ex00foo en 404", val("ex00_404").split("|")[0], "404")

    for key in ("ex01_django", "ex01_django_slash", "ex01_affichage", "ex01_templates"):
        r.equals(f"{key}: 200", val(key).split("|")[0], "200")
        r.equals(f"{key}: titre", val(f"{key}_title"), "yes")
    r.equals("ex01: style1.css utilise", val("style1_used"), "yes")
    r.equals("ex01: style2.css sur la page templates", val("style2_used"), "yes")
    r.equals("ex01: style1.css absente de la page templates",
             val("style1_absent_on_templates"), "yes")

    r.equals("ex02: formulaire present", val("ex02_form"), "yes")
    r.equals("ex02: POST 200", val("ex02_post"), "200")
    r.equals("ex02: entree dans l'historique", val("ex02_history"), "yes")
    r.equals("ex02: fichier de log cree", val("ex02_logfile"), "yes")
    r.equals("ex02: entree dans le fichier de log", val("ex02_logcontent"), "yes")

    r.equals("ex03: 4 balises <th>", val("ex03_th"), "4|4")
    r.equals("ex03: 51 lignes <tr>", val("ex03_tr"), "51|51")
    r.equals("ex03: 200 cellules <td>", val("ex03_td"), "200|200")
    r.equals("ex03: 200 nuances toutes differentes", val("ex03_shades"), "200|200")

    return r.summary()


if __name__ == "__main__":
    raise SystemExit(main())
