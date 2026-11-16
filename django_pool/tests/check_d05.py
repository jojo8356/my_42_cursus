#!/usr/bin/env python3
"""Tests du jour d05 - SQL (projet d42), exécutés sur SQLite."""

from __future__ import annotations

import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _harness import POOL_DIR, Report, make_tmp, run_py  # noqa: E402

PROJECT = POOL_DIR / "d05_sql" / "d42"

PROBE = r'''
import os
import re
import sys
import django

sys.path.insert(0, sys.argv[1])
os.environ['DJANGO_SETTINGS_MODULE'] = 'd42.settings'
os.environ['DJANGO_DB_PATH'] = sys.argv[2]
django.setup()

from django.core.management import call_command
from django.test import Client

call_command('migrate', verbosity=0, interactive=False)

c = Client()
out = []


def get(label, url):
    r = c.get(url, follow=True)
    body = r.content.decode('utf-8', 'replace')
    out.append(f"{label}|{r.status_code}|{body.strip()[:400]}")
    return r, body


def post(label, url, data):
    r = c.post(url, data)
    body = r.content.decode('utf-8', 'replace')
    out.append(f"{label}|{r.status_code}|{body.strip()[:400]}")
    return r, body


# ex00
_, b = get('ex00_init', '/ex00/init')
out.append('ex00_ok|' + (b.strip() == 'OK' and 'yes' or 'no'))

# ex01 : populate puis display
_, b = post('ex01_populate', '/ex01/populate', {})
out.append('ex01_populate_ok|' + (b.count('OK') >= 7 and 'yes' or 'no'))
_, b = get('ex01_display', '/ex01/display')
out.append('ex01_display_ok|' + ('A New Hope' in b and 'yes' or 'no'))

# ex02
_, b = get('ex02_init', '/ex02/init')
_, b = post('ex02_populate', '/ex02/populate', {})
out.append('ex02_populate_ok|' + (b.count('OK') >= 7 and 'yes' or 'no'))
_, b = get('ex02_display', '/ex02/display')
out.append('ex02_display_ok|' + ('A New Hope' in b and 'yes' or 'no'))

# ex03
_, b = post('ex03_populate', '/ex03/populate', {})
out.append('ex03_populate_ok|' + (b.count('OK') >= 7 and 'yes' or 'no'))
_, b = get('ex03_display', '/ex03/display')
out.append('ex03_display_ok|' + ('A New Hope' in b and 'yes' or 'no'))

# ex04
_, b = get('ex04_init', '/ex04/init')
_, b = post('ex04_populate', '/ex04/populate', {})
out.append('ex04_populate_ok|' + (b.count('OK') >= 7 and 'yes' or 'no'))
_, b = get('ex04_display', '/ex04/display')
out.append('ex04_display_ok|' + ('A New Hope' in b and 'yes' or 'no'))
_, b = get('ex04_remove', '/ex04/remove')
out.append('ex04_remove_form|' + ('<select' in b and 'remove' in b and 'yes' or 'no'))
_, b = post('ex04_remove_post', '/ex04/remove', {'title': 'A New Hope', 'remove': 'remove'})
out.append('ex04_removed|' + ('A New Hope' not in b and 'yes' or 'no'))
_, b = get('ex04_display_after', '/ex04/display')
out.append('ex04_display_after|' + ('A New Hope' not in b and 'yes' or 'no'))

# ex05
_, b = post('ex05_populate', '/ex05/populate', {})
out.append('ex05_populate_ok|' + (b.count('OK') >= 7 and 'yes' or 'no'))
_, b = get('ex05_display', '/ex05/display')
out.append('ex05_display_ok|' + ('A New Hope' in b and 'yes' or 'no'))
_, b = post('ex05_remove_post', '/ex05/remove', {'title': 'A New Hope', 'remove': 'remove'})
out.append('ex05_removed|' + ('A New Hope' not in b and 'yes' or 'no'))

# ex06
_, b = get('ex06_init', '/ex06/init')
_, b = post('ex06_populate', '/ex06/populate', {})
out.append('ex06_populate_ok|' + (b.count('OK') >= 7 and 'yes' or 'no'))
_, b = get('ex06_display', '/ex06/display')
out.append('ex06_display_ok|' + ('A New Hope' in b and 'yes' or 'no'))
out.append('ex06_has_created|' + ('created' in b.lower() and 'yes' or 'no'))
out.append('ex06_has_updated|' + ('updated' in b.lower() and 'yes' or 'no'))
_, b = post('ex06_update', '/ex06/update', {'title': 'A New Hope', 'opening_crawl': 'OPENING CRAWL EDIT'})
out.append('ex06_updated|' + ('OPENING CRAWL EDIT' in b and 'yes' or 'no'))
_, b = get('ex06_display_after', '/ex06/display')
out.append('ex06_updated_after|' + ('OPENING CRAWL EDIT' in b and 'yes' or 'no'))

# ex07
_, b = post('ex07_populate', '/ex07/populate', {})
out.append('ex07_populate_ok|' + (b.count('OK') >= 7 and 'yes' or 'no'))
_, b = get('ex07_display', '/ex07/display')
out.append('ex07_display_ok|' + ('A New Hope' in b and 'yes' or 'no'))
_, b = post('ex07_update', '/ex07/update', {'title': 'A New Hope', 'opening_crawl': 'CRAWL ORM'})
out.append('ex07_updated|' + ('CRAWL ORM' in b and 'yes' or 'no'))
_, b = get('ex07_display_after', '/ex07/display')
out.append('ex07_updated_after|' + ('CRAWL ORM' in b and 'yes' or 'no'))

# ex08
_, b = get('ex08_init', '/ex08/init')
_, b = post('ex08_populate', '/ex08/populate', {})
out.append('ex08_populate_ok|' + ('OK' in b and 'yes' or 'no'))
_, b = get('ex08_display', '/ex08/display')
out.append('ex08_display_ok|' + ('No data available' not in b and 'yes' or 'no'))

# ex09
_, b = get('ex09_populate', '/ex09/populate')
out.append('ex09_populate_ok|' + ('OK' in b and 'yes' or 'no'))
_, b = get('ex09_display', '/ex09/display')
out.append('ex09_display_ok|' + ('No data available' not in b and 'yes' or 'no'))

# ex10
_, b = get('ex10_populate', '/ex10/populate')
out.append('ex10_populate_ok|' + ('OK' in b and 'yes' or 'no'))
_, b = get('ex10_index', '/ex10/')
out.append('ex10_index|' + ('<form' in b and 'yes' or 'no'))
_, b = post('ex10_query', '/ex10/', {
    'min_date': '1977-01-01', 'max_date': '2016-01-01',
    'min_diameter': '0', 'gender': 'male'})
out.append('ex10_query_ok|' + ('A New Hope' in b and 'yes' or 'no'))

print('##')
print('\n'.join(out))
'''


def main() -> int:
    r = Report("d05 - SQL")
    tmp = make_tmp()
    probe = tmp / "probe.py"
    probe.write_text(PROBE)
    db = tmp / "db.sqlite3"
    rc, out, err = run_py(probe, [str(PROJECT), str(db)], timeout=240)
    if rc != 0:
        r.fail("d42: probe Django", err[-900:])
        return r.summary()
    body = out.split("##\n", 1)[1] if "##\n" in out else ""
    results = {}
    for line in body.strip().splitlines():
        if "|" in line:
            key, _, value = line.partition("|")
            results[key] = value

    def val(key):
        return results.get(key, "<absent>")

    def status(key):
        return val(key).split("|")[0]

    r.equals("ex00: init OK", val("ex00_ok"), "yes")

    r.equals("ex01: populate", val("ex01_populate_ok"), "yes")
    r.equals("ex01: display", val("ex01_display_ok"), "yes")

    r.equals("ex02: populate", val("ex02_populate_ok"), "yes")
    r.equals("ex02: display", val("ex02_display_ok"), "yes")

    r.equals("ex03: populate", val("ex03_populate_ok"), "yes")
    r.equals("ex03: display", val("ex03_display_ok"), "yes")

    r.equals("ex04: populate", val("ex04_populate_ok"), "yes")
    r.equals("ex04: display", val("ex04_display_ok"), "yes")
    r.equals("ex04: formulaire remove", val("ex04_remove_form"), "yes")
    r.equals("ex04: suppression", val("ex04_removed"), "yes")
    r.equals("ex04: display apres suppression", val("ex04_display_after"), "yes")

    r.equals("ex05: populate", val("ex05_populate_ok"), "yes")
    r.equals("ex05: display", val("ex05_display_ok"), "yes")
    r.equals("ex05: suppression", val("ex05_removed"), "yes")

    r.equals("ex06: populate", val("ex06_populate_ok"), "yes")
    r.equals("ex06: display", val("ex06_display_ok"), "yes")
    r.equals("ex06: colonnes created/updated", val("ex06_has_created") + "|"
             + val("ex06_has_updated"), "yes|yes")
    r.equals("ex06: update", val("ex06_updated_after"), "yes")

    r.equals("ex07: populate", val("ex07_populate_ok"), "yes")
    r.equals("ex07: display", val("ex07_display_ok"), "yes")
    r.equals("ex07: update", val("ex07_updated_after"), "yes")

    r.equals("ex08: populate", val("ex08_populate_ok"), "yes")
    r.equals("ex08: display", val("ex08_display_ok"), "yes")

    r.equals("ex09: populate", val("ex09_populate_ok"), "yes")
    r.equals("ex09: display", val("ex09_display_ok"), "yes")

    r.equals("ex10: populate", val("ex10_populate_ok"), "yes")
    r.equals("ex10: index", val("ex10_index"), "yes")
    r.equals("ex10: requete", val("ex10_query_ok"), "yes")

    return r.summary()


if __name__ == "__main__":
    raise SystemExit(main())
