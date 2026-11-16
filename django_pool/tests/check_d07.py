#!/usr/bin/env python3
"""Tests du jour d07 - Avancé (projet articles_project)."""

from __future__ import annotations

import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _harness import POOL_DIR, Report, make_tmp, run_py  # noqa: E402

PROJECT = POOL_DIR / "d07_advanced"

PROBE = r'''
import os
import re
import sys

import django

sys.path.insert(0, sys.argv[1])
os.environ['DJANGO_SETTINGS_MODULE'] = 'articles_project.settings'
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


# --- fixtures / modele
from articles_app.models import Article, UserFavouriteArticle
emit('model_article_fields', all(f in [x.name for x in Article._meta.fields]
                                 for f in ('title', 'author', 'created', 'synopsis', 'content')))
emit('model_fav_fields', all(f in [x.name for x in UserFavouriteArticle._meta.fields]
                             for f in ('user', 'article')))
emit('str_article', Article(title='T').__str__() == 'T')

# --- ex00
c = Client()
r = c.get('/')
emit('home_redirect', r.status_code in (301, 302))
b = body(c.get('/en/articles/'))
emit('articles_200', c.get('/en/articles/').status_code == 200)
emit('articles_table', '<table' in b)
emit('articles_header', '<th>' in b)
emit('articles_no_content', 'Lorem ipsum dolor sit amet, consectetur' not in b)
r = c.get('/en/login/')
b = body(r)
emit('login_200', r.status_code == 200)
emit('login_form_post', 'method="post"' in b)
emit('login_fields', 'name="username"' in b and 'name="password"' in b)

# --- fixtures fournies (5 articles, 3 utilisateurs)
emit('five_articles', Article.objects.count() >= 5)
emit('three_authors', Article.objects.values('author').distinct().count() >= 3)

# --- ex01
u1 = User.objects.create_user('alice2', password='secret123')
u2 = User.objects.create_user('bruno2', password='secret123')
a1 = Article.objects.create(title='Article un', author=u1, synopsis='Synopsis un',
                            content='Contenu un')
a2 = Article.objects.create(title='Article deux', author=u1, synopsis='Synopsis deux',
                            content='Contenu deux')
a3 = Article.objects.create(title='Article trois', author=u2, synopsis='Synopsis trois',
                            content='Contenu trois')
c.login(username='alice2', password='secret123')
b = body(c.get('/en/publications/'))
emit('publications_200', c.get('/en/publications/').status_code == 200)
emit('publications_own', 'Article un' in b and 'Article deux' in b)
emit('publications_not_others', 'Article trois' not in b)
emit('publications_fields', 'Synopsis un' in b)
emit('publications_detail_link', '/en/articles/%d/' % a1.pk in b)
b = body(c.get('/en/articles/%d/' % a1.pk))
emit('detail_200', c.get('/en/articles/%d/' % a1.pk).status_code == 200)
emit('detail_all_fields', 'Article un' in b and 'Synopsis un' in b and 'Contenu un' in b)
b = body(c.get('/en/articles/'))
emit('articles_detail_link', '/en/articles/%d/' % a1.pk in b)
emit('logout_link', '/en/logout/' in b)
emit('logged_as', 'alice2' in b)
# favoris
UserFavouriteArticle.objects.create(user=u1, article=a1)
UserFavouriteArticle.objects.create(user=u1, article=a3)
b = body(c.get('/en/favourites/'))
emit('favourites_200', c.get('/en/favourites/').status_code == 200)
emit('favourites_two', 'Article un' in b and 'Article trois' in b)
emit('favourites_links', '/en/articles/%d/' % a1.pk in b and '/en/articles/%d/' % a3.pk in b)
c.get('/en/logout/')
emit('logout_ok', '_auth_user_id' not in c.session)

# --- ex02
b = body(c.get('/en/register/'))
emit('register_url', c.get('/en/register/').status_code == 200)
emit('register_fields', 'name="username"' in b and 'name="password1"' in b
     and 'name="password2"' in b)
r = c.post('/en/register/', {'username': 'carol', 'password1': 'secret123',
                             'password2': 'secret123'})
emit('register_ok', User.objects.filter(username='carol').exists())
c.login(username='alice2', password='secret123')
b = body(c.get('/en/publications/'))
emit('publish_link', '/en/publish/' in b)
r = c.post('/en/publish/', {'title': 'Nouvel article', 'synopsis': 'Nouveau',
                            'content': 'Contenu nouvel'})
emit('publish_ok', Article.objects.filter(title='Nouvel article', author=u1).exists())
emit('publish_author_hidden', 'name="author"' not in body(c.get('/en/publish/')))
b = body(c.get('/en/articles/%d/' % a1.pk))
emit('fav_form_in_detail', 'method="post"' in b)
emit('fav_no_visible_field', ('name="article"' not in b) or ('type="hidden"' in b))
c.login(username='bruno2', password='secret123')
r = c.post('/en/articles/%d/favourite/' % a3.pk, {'article': a3.pk})
emit('fav_created', UserFavouriteArticle.objects.filter(user=u2, article=a3).exists())

# --- ex03 : menu et templates
c.logout()
pages = ['/en/articles/', '/en/login/', '/en/register/']
for p in pages:
    b = body(c.get(p))
    emit('menu_' + p.replace('/', '_').strip('_'),
         'Last Articles' in b and 'Home' in b and '/en/register/' in b)
    emit('login_form_in_menu_' + p.replace('/', '_').strip('_'),
         'name="username"' in b and 'name="password"' in b)
b = body(c.get('/en/articles/'))
emit('synopsis_truncated', '...' in b or '…' in b)
emit('since_column', 'ago' in b or 'Since' in b)

# --- ex04 : bootstrap
b = body(c.get('/en/articles/'))
emit('bootstrap_css', 'bootstrap' in b.lower())

# --- ex05 : i18n
r = c.get('/en/articles/')
be = body(r)
r = c.get('/fr/articles/')
bf = body(r)
emit('i18n_en_200', r.status_code == 200)
emit('i18n_en_title', 'Articles' in be)
emit('i18n_fr_title', 'Articles' in bf)
emit('i18n_lang_link', '/fr/articles/' in be or '/en/articles/' in bf)

# --- ex06 : tests du projet
import subprocess
proj = sys.argv[1]
r = subprocess.run([sys.executable, 'manage.py', 'test', 'articles_app', '-v', '2'],
                   cwd=proj, capture_output=True, text=True)
emit('django_tests_run', r.returncode == 0)
emit('django_tests_named', 'test_' in r.stderr)

print('##')
print('\n'.join(out))
'''


def main() -> int:
    r = Report("d07 - Avancé")
    tmp = make_tmp()
    probe = tmp / "probe.py"
    probe.write_text(PROBE)
    db = tmp / "db.sqlite3"
    rc, out, err = run_py(probe, [str(PROJECT), str(db)], timeout=300)
    if rc != 0:
        r.fail("articles_project: probe Django", err[-900:])
        return r.summary()
    bodytxt = out.split("##\n", 1)[1] if "##\n" in out else ""
    res = {}
    for line in bodytxt.strip().splitlines():
        if "|" in line:
            k, _, v = line.partition("|")
            res[k] = v

    def val(k):
        return res.get(k, "<absent>")

    r.equals("ex00: champs du modele Article", val("model_article_fields"), "yes")
    r.equals("ex00: champs du modele UserFavouriteArticle",
             val("model_fav_fields"), "yes")
    r.equals("ex00: __str__ Article", val("str_article"), "yes")
    r.equals("ex00: Home redirige", val("home_redirect"), "yes")
    r.equals("ex00: page Articles 200", val("articles_200"), "yes")
    r.equals("ex00: tableau HTML", val("articles_table"), "yes")
    r.equals("ex00: header du tableau", val("articles_header"), "yes")
    r.equals("ex00: contenu non affiche", val("articles_no_content"), "yes")
    r.equals("ex00: page Login 200", val("login_200"), "yes")
    r.equals("ex00: formulaire POST", val("login_form_post"), "yes")
    r.equals("ex00: champs login", val("login_fields"), "yes")
    r.equals("ex00: au moins 5 articles", val("five_articles"), "yes")
    r.equals("ex00: au moins 3 auteurs", val("three_authors"), "yes")

    r.equals("ex01: Publications 200", val("publications_200"), "yes")
    r.equals("ex01: articles de l'utilisateur", val("publications_own"), "yes")
    r.equals("ex01: pas les articles des autres",
             val("publications_not_others"), "yes")
    r.equals("ex01: champs affiches", val("publications_fields"), "yes")
    r.equals("ex01: lien vers Detail", val("publications_detail_link"), "yes")
    r.equals("ex01: Detail 200", val("detail_200"), "yes")
    r.equals("ex01: Detail tous les champs", val("detail_all_fields"), "yes")
    r.equals("ex01: liens Detail dans Articles", val("articles_detail_link"), "yes")
    r.equals("ex01: lien Logout", val("logout_link"), "yes")
    r.equals("ex01: logged as <user>", val("logged_as"), "yes")
    r.equals("ex01: Favourites 200", val("favourites_200"), "yes")
    r.equals("ex01: deux favoris", val("favourites_two"), "yes")
    r.equals("ex01: liens des favoris", val("favourites_links"), "yes")
    r.equals("ex01: logout fonctionnel", val("logout_ok"), "yes")

    r.equals("ex02: page register", val("register_url"), "yes")
    r.equals("ex02: champs register", val("register_fields"), "yes")
    r.equals("ex02: inscription reussie", val("register_ok"), "yes")
    r.equals("ex02: lien Publish", val("publish_link"), "yes")
    r.equals("ex02: publication reussie", val("publish_ok"), "yes")
    r.equals("ex02: champ author cache", val("publish_author_hidden"), "yes")
    r.equals("ex02: formulaire favori dans Detail", val("fav_form_in_detail"), "yes")
    r.equals("ex02: champ article cache", val("fav_no_visible_field"), "yes")
    r.equals("ex02: favori cree", val("fav_created"), "yes")

    r.equals("ex03: menu sur articles", val("menu_en_articles"), "yes")
    r.equals("ex03: menu sur login", val("menu_en_login"), "yes")
    r.equals("ex03: menu sur register", val("menu_en_register"), "yes")
    r.equals("ex03: formulaire login dans le menu (articles)",
             val("login_form_in_menu_en_articles"), "yes")
    r.equals("ex03: formulaire login dans le menu (login)",
             val("login_form_in_menu_en_login"), "yes")
    r.equals("ex03: formulaire login dans le menu (register)",
             val("login_form_in_menu_en_register"), "yes")
    r.equals("ex03: synopsis tronque", val("synopsis_truncated"), "yes")
    r.equals("ex03: colonne depuis", val("since_column"), "yes")

    r.equals("ex04: bootstrap", val("bootstrap_css"), "yes")

    r.equals("ex05: page en 200", val("i18n_en_200"), "yes")
    r.equals("ex05: contenu anglais", val("i18n_en_title"), "yes")
    r.equals("ex05: contenu francais", val("i18n_fr_title"), "yes")
    r.equals("ex05: lien de changement de langue", val("i18n_lang_link"), "yes")

    r.equals("ex06: tests Django executés", val("django_tests_run"), "yes")
    r.equals("ex06: tests nommes", val("django_tests_named"), "yes")

    return r.summary()


if __name__ == "__main__":
    raise SystemExit(main())
