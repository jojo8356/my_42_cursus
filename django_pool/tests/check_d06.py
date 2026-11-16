#!/usr/bin/env python3
"""Tests du jour d06 - Sessions (projet ex)."""

from __future__ import annotations

import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _harness import POOL_DIR, Report, make_tmp, run_py  # noqa: E402

PROJECT = POOL_DIR / "d06_sessions" / "ex"

PROBE = r'''
import json
import os
import re
import sys
import time

import django

sys.path.insert(0, sys.argv[1])
os.environ['DJANGO_SETTINGS_MODULE'] = 'ex_project.settings'
os.environ['DJANGO_DB_PATH'] = sys.argv[2]
django.setup()

from django.conf import settings
from django.core.management import call_command
from django.test import Client
from django.contrib.auth import get_user_model

call_command('migrate', verbosity=0, interactive=False)
User = get_user_model()
out = []


def emit(label, ok):
    out.append(label + '|' + ('yes' if ok else 'no'))


def body(resp):
    return resp.content.decode('utf-8', 'replace')


# --- ex00 : sessions anonymes
c = Client()
r = c.get('/')
b = body(r)
emit('home_200', r.status_code == 200)
names = getattr(settings, 'ANONYMOUS_NAMES', [])
emit('settings_10_names', len(names) == 10)
emit('settings_duration_42', getattr(settings, 'ANONYMOUS_NAME_DURATION', None) == 42)
nav = re.search(r'<nav.*?</nav>', b, re.S)
emit('nav_present', bool(nav))
m = re.search(r'Hello\s+([A-Za-z0-9_-]+)!', b)
emit('hello_message', bool(m))
if m:
    out.append('name|' + m.group(1))
    emit('name_in_list', m.group(1) in names)
# persistance sur la meme session
b2 = body(c.get('/'))
m2 = re.search(r'Hello\s+([A-Za-z0-9_-]+)!', b2)
emit('name_stable', bool(m and m2 and m.group(1) == m2.group(1)))
# expiration apres 42 secondes (on avance l'horloge de la session)
c2 = Client()
b3 = body(c2.get('/'))
m3 = re.search(r'Hello\s+([A-Za-z0-9_-]+)!', b3)
sess = c2.session
sess['anonymous_name_set_at'] = time.time() - 43
sess.save()
c2.cookies[settings.SESSION_COOKIE_NAME] = sess.session_key
b4 = body(c2.get('/'))
m4 = re.search(r'Hello\s+([A-Za-z0-8_-]+)!', b4)
emit('name_expires', bool(m3 and m4 and m3.group(1) != m4.group(1)))
# deux visiteurs different
c3 = Client()
m5 = re.search(r'Hello\s+([A-Za-z0-9_-]+)!', body(c3.get('/')))
emit('different_visitors', bool(m3 and m5 and m3.group(1) != m5.group(1)))

# --- ex01 : authentification
r = c.get('/')
b = body(r)
emit('nav_site_name', 'Life Pro Tips' in b)
emit('nav_register_link', '/register/' in b)
emit('nav_login_link', '/login/' in b)
emit('nav_home_link', 'href="/"' in b or 'href="/"' in b)
r = c.get('/register/')
b = body(r)
emit('register_200', r.status_code == 200)
emit('register_3_fields', b.count('name="username"') == 1 and 'name="password"' in b
     and 'name="password2"' in b)
emit('register_password_hidden', 'type="password"' in b)
r = c.get('/login/')
b = body(r)
emit('login_200', r.status_code == 200)
emit('login_2_fields', 'name="username"' in b and 'name="password"' in b)

# erreurs d'inscription
r = c.post('/register/', {'username': '', 'password': '', 'password2': ''})
emit('register_empty_rejected', r.status_code == 200 and
     ('required' in body(r).lower() or 'requis' in body(r).lower() or 'error' in body(r).lower()))
r = c.post('/register/', {'username': 'bob', 'password': 'secret123', 'password2': 'other'})
emit('register_mismatch_rejected', r.status_code == 200 and
     ('correspondent' in body(r).lower() or 'diff' in body(r).lower() or 'error' in body(r).lower()))
r = c.post('/register/', {'username': 'bob', 'password': 'secret123', 'password2': 'secret123'})
emit('register_ok', User.objects.filter(username='bob').exists())
c.logout()
r = c.post('/register/', {'username': 'bob', 'password': 'secret123', 'password2': 'secret123'})
rb = body(r)
emit('register_duplicate_rejected', r.status_code == 200 and
     ('existe' in rb.lower() or 'used' in rb.lower() or 'utilis' in rb.lower()))

# login
r = c.post('/login/', {'username': 'bob', 'password': 'wrongpass'})
emit('login_bad_rejected', r.status_code == 200 and 'incorrect' in body(r).lower()
     or 'invalide' in body(r).lower() or 'error' in body(r).lower())
c.logout()
r = c.post('/login/', {'username': 'bob', 'password': 'secret123'})
emit('login_ok_redirect', r.status_code in (301, 302))
emit('login_logged', '_auth_user_id' in c.session)
b = body(c.get('/'))
emit('logged_no_register', '/register/' not in b)
emit('logged_no_login', '/login/' not in b)
emit('logged_logout_link', '/logout/' in b)
emit('logged_username_in_nav', 'bob' in b)
emit('logged_no_hello', 'Hello ' not in b)
r = c.get('/register/')
emit('logged_register_redirect', r.status_code in (301, 302))
r = c.get('/login/')
emit('logged_login_redirect', r.status_code in (301, 302))
r = c.get('/logout/')
emit('logout_redirect', r.status_code in (301, 302))
emit('logout_done', '_auth_user_id' not in c.session)

# --- ex02 : Tips
from tips_app.models import Tip
emit('tip_model_fields', all(f in [x.name for x in Tip._meta.fields]
                             for f in ('content', 'author', 'date')))
c.login(username='bob', password='secret123')
r = c.post('/', {'content': 'Premier tip de test'})
emit('tip_create_ok', Tip.objects.filter(content='Premier tip de test').exists())
b = body(c.get('/'))
emit('tip_listed', 'Premier tip de test' in b)
emit('tip_author_shown', 'bob' in b)
emit('tip_date_shown', bool(re.search(r'\d{2}/\d{2}/\d{4}', b)))
c.logout()
b = body(c.get('/'))
emit('anon_no_form', 'name="content"' not in b)
r = c.post('/', {'content': 'Tip anonyme'})
emit('anon_cannot_create', not Tip.objects.filter(content='Tip anonyme').exists())

# --- ex03 : votes
c.login(username='bob', password='secret123')
tip = Tip.objects.first()
up0 = tip.upvote_count
down0 = tip.downvote_count
emit('initial_zero', up0 == 0 and down0 == 0)
c.get('/vote/%d/upvote/' % tip.id)
tip.refresh_from_db()
emit('upvote_plus', tip.upvote_count == up0 + 1)
c.get('/vote/%d/upvote/' % tip.id)
tip.refresh_from_db()
emit('upvote_toggle', tip.upvote_count == up0)
c.get('/vote/%d/upvote/' % tip.id)
c.get('/vote/%d/downvote/' % tip.id)
tip.refresh_from_db()
emit('up_then_down', tip.upvote_count == up0 and tip.downvote_count == down0 + 1)
c.get('/vote/%d/downvote/' % tip.id)
tip.refresh_from_db()
emit('down_toggle', tip.downvote_count == down0)
c.logout()
r = c.get('/vote/%d/upvote/' % tip.id)
emit('anon_vote_refused', r.status_code in (301, 302, 403))
r = c.get('/delete/%d/' % tip.id)
emit('anon_delete_refused', r.status_code in (301, 302, 403))

# --- ex04/05 : permissions
from django.contrib.auth.models import Permission
carol = User.objects.create_user('carol', password='secret123')
bob = User.objects.get(username='bob')
tip2 = Tip.objects.create(content='Tip de bob', author=bob)
c.login(username='carol', password='secret123')
r = c.get('/delete/%d/' % tip2.id)
emit('other_delete_refused', r.status_code in (301, 302, 403))
emit('other_delete_intact', Tip.objects.filter(pk=tip2.id).exists())
carol.user_permissions.add(Permission.objects.get(codename='delete_tip'))
c.login(username='carol', password='secret123')
r = c.get('/delete/%d/' % tip2.id)
emit('perm_delete_ok', not Tip.objects.filter(pk=tip2.id).exists())
emit('can_downvote_permission', Permission.objects.filter(codename='can_downvote').exists())

# --- ex06 : reputation
user_cls = type(carol)
emit('custom_user_class', user_cls.__module__ != 'django.contrib.auth.models'
     or user_cls.__name__ != 'User')
emit('reputation_field', 'reputation' in [f.name for f in user_cls._meta.fields])
emit('reputation_start_zero', carol.reputation == 0)
tip3 = Tip.objects.create(content='Tip rep', author=bob)
c.login(username='carol', password='secret123')
c.get('/vote/%d/upvote/' % tip3.id)
bob.refresh_from_db()
emit('upvote_reputation', bob.reputation == 5)
c.get('/vote/%d/upvote/' % tip3.id)
bob.refresh_from_db()
emit('upvote_cancel', bob.reputation == 0)
carol.user_permissions.add(Permission.objects.get(codename='can_downvote'))
c.get('/vote/%d/downvote/' % tip3.id)
bob.refresh_from_db()
emit('downvote_reputation', bob.reputation == -2)
# 4 upvotes recus -> 18 points (>= 15) -> permission can_downvote
t4 = Tip.objects.create(content='Tip rep 4', author=bob)
t5 = Tip.objects.create(content='Tip rep 5', author=bob)
t6 = Tip.objects.create(content='Tip rep 6', author=bob)
for t in (tip3, t4, t5, t6):
    c.get('/vote/%d/upvote/' % t.id)
bob.refresh_from_db()
emit('reputation_15', bob.reputation >= 15)
emit('reputation_grants_downvote', bob.has_perm('tips_app.can_downvote'))
b = body(c.get('/'))
emit('reputation_displayed', bool(re.search(r'\(\s*-?\d+\s*\)', b)))

print('##')
print('\n'.join(out))
'''


def main() -> int:
    r = Report("d06 - Sessions")
    tmp = make_tmp()
    probe = tmp / "probe.py"
    probe.write_text(PROBE)
    db = tmp / "db.sqlite3"
    rc, out, err = run_py(probe, [str(PROJECT), str(db)], timeout=240)
    if rc != 0:
        r.fail("ex: probe Django", err[-900:])
        return r.summary()
    body = out.split("##\n", 1)[1] if "##\n" in out else ""
    res = {}
    for line in body.strip().splitlines():
        if "|" in line:
            k, _, v = line.partition("|")
            res[k] = v

    def val(k):
        return res.get(k, "<absent>")

    r.equals("ex00: 10 noms dans settings", val("settings_10_names"), "yes")
    r.equals("ex00: duree 42s", val("settings_duration_42"), "yes")
    r.equals("ex00: page d'accueil 200", val("home_200"), "yes")
    r.equals("ex00: message Hello <nom>!", val("hello_message"), "yes")
    r.equals("ex00: nom dans la liste", val("name_in_list"), "yes")
    r.equals("ex00: nom stable pendant 42s", val("name_stable"), "yes")
    r.equals("ex00: nom redefini apres 42s", val("name_expires"), "yes")
    r.equals("ex00: noms differents entre visiteurs", val("different_visitors"), "yes")
    r.equals("ex00: element nav present", val("nav_present"), "yes")

    r.equals("ex01: nom du site dans le nav", val("nav_site_name"), "yes")
    r.equals("ex01: lien register", val("nav_register_link"), "yes")
    r.equals("ex01: lien login", val("nav_login_link"), "yes")
    r.equals("ex01: page register 200", val("register_200"), "yes")
    r.equals("ex01: 3 champs inscription", val("register_3_fields"), "yes")
    r.equals("ex01: mot de passe masque", val("register_password_hidden"), "yes")
    r.equals("ex01: page login 200", val("login_200"), "yes")
    r.equals("ex01: 2 champs connexion", val("login_2_fields"), "yes")
    r.equals("ex01: inscription vide rejetee", val("register_empty_rejected"), "yes")
    r.equals("ex01: mots de passe differents rejetes",
             val("register_mismatch_rejected"), "yes")
    r.equals("ex01: inscription reussie", val("register_ok"), "yes")
    r.equals("ex01: doublon rejete", val("register_duplicate_rejected"), "yes")
    r.equals("ex01: mauvais mot de passe rejete", val("login_bad_rejected"), "yes")
    r.equals("ex01: connexion redirige", val("login_ok_redirect"), "yes")
    r.equals("ex01: utilisateur connecte", val("login_logged"), "yes")
    r.equals("ex01: plus de lien register", val("logged_no_register"), "yes")
    r.equals("ex01: plus de lien login", val("logged_no_login"), "yes")
    r.equals("ex01: lien logout present", val("logged_logout_link"), "yes")
    r.equals("ex01: nom utilisateur dans le nav", val("logged_username_in_nav"), "yes")
    r.equals("ex01: plus de Hello anonyme", val("logged_no_hello"), "yes")
    r.equals("ex01: register redirige une fois connecte",
             val("logged_register_redirect"), "yes")
    r.equals("ex01: login redirige une fois connecte", val("logged_login_redirect"), "yes")
    r.equals("ex01: logout fonctionnel", val("logout_redirect") + "|" + val("logout_done"),
             "yes|yes")

    r.equals("ex02: champs du modele Tip", val("tip_model_fields"), "yes")
    r.equals("ex02: creation de tip", val("tip_create_ok"), "yes")
    r.equals("ex02: tip affiche", val("tip_listed"), "yes")
    r.equals("ex02: auteur affiche", val("tip_author_shown"), "yes")
    r.equals("ex02: date affichee", val("tip_date_shown"), "yes")
    r.equals("ex02: pas de formulaire anonyme", val("anon_no_form"), "yes")
    r.equals("ex02: anonyme ne peut pas creer", val("anon_cannot_create"), "yes")

    r.equals("ex03: 0 votes au depart", val("initial_zero"), "yes")
    r.equals("ex03: upvote", val("upvote_plus"), "yes")
    r.equals("ex03: upvote annulable", val("upvote_toggle"), "yes")
    r.equals("ex03: upvote puis downvote", val("up_then_down"), "yes")
    r.equals("ex03: downvote annulable", val("down_toggle"), "yes")
    r.equals("ex03: vote anonyme refuse", val("anon_vote_refused"), "yes")
    r.equals("ex03: suppression anonyme refusee", val("anon_delete_refused"), "yes")

    r.equals("ex04: suppression d'autrui refusee", val("other_delete_refused"), "yes")
    r.equals("ex04: tip intact apres refus", val("other_delete_intact"), "yes")
    r.equals("ex04: suppression avec permission", val("perm_delete_ok"), "yes")
    r.equals("ex05: permission can_downvote", val("can_downvote_permission"), "yes")

    r.equals("ex06: classe utilisateur personnalisee", val("custom_user_class"), "yes")
    r.equals("ex06: champ reputation", val("reputation_field"), "yes")
    r.equals("ex06: reputation initiale 0", val("reputation_start_zero"), "yes")
    r.equals("ex06: upvote rapporte 5", val("upvote_reputation"), "yes")
    r.equals("ex06: annulation upvote", val("upvote_cancel"), "yes")
    r.equals("ex06: downvote retire 2", val("downvote_reputation"), "yes")
    r.equals("ex06: 15 points de reputation", val("reputation_15"), "yes")
    r.equals("ex06: permission accordee a 15 points",
             val("reputation_grants_downvote"), "yes")
    r.equals("ex06: reputation affichee entre parentheses",
             val("reputation_displayed"), "yes")

    return r.summary()


if __name__ == "__main__":
    raise SystemExit(main())
