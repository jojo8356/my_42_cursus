#!/usr/bin/env python3
"""Tests du projet final d08 (d09 : account AJAX + chat Websockets)."""

from __future__ import annotations

import asyncio
import json
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _harness import POOL_DIR, Report, make_tmp, run_py  # noqa: E402

PROJECT = POOL_DIR / "d08_final" / "d09"

PROBE = r'''
import asyncio
import json
import os
import sys

import django

sys.path.insert(0, sys.argv[1])
os.environ['DJANGO_SETTINGS_MODULE'] = 'd09.settings'
os.environ['DJANGO_DB_PATH'] = sys.argv[2]
django.setup()

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


# --- ex00 : account AJAX
c = Client()
b = body(c.get('/account/'))
emit('account_200', c.get('/account/').status_code == 200)
emit('account_form_anon', 'id="login-ajax"' in b and 'name="username"' in b
     and 'name="password"' in b)
emit('account_hidden_anon', 'id="logged-in" style="display:none;"' in b)
emit('account_jquery', 'jquery' in b.lower())
r = c.post('/login/', {'username': 'x', 'password': 'y'})
emit('ajax_login_405_en_get', c.get('/login/').status_code in (405, 404, 302))
u = User.objects.create_user('alice', password='secret123')
r = c.post('/login/', {'username': 'alice', 'password': 'wrong'})
j = r.json()
emit('ajax_login_ko', r.status_code == 200 and j.get('success') is False and j.get('errors'))
r = c.post('/login/', {'username': 'alice', 'password': 'secret123'})
j = r.json()
emit('ajax_login_ok', r.status_code == 200 and j.get('success') is True
     and j.get('username') == 'alice')
b = body(c.get('/account/'))
emit('account_logged', 'Logged as' in b and 'alice' in b and 'id="logout-btn"' in b)
emit('account_logged_form_hidden', 'id="login-form" style="display:none;"' in b)
r = c.post('/logout/')
emit('ajax_logout_ok', r.status_code == 200 and r.json().get('success') is True)
b = body(c.get('/account/'))
emit('account_back_anon', 'id="logged-in" style="display:none;"' in b)

# --- ex01 : chat
from chat.models import ChatRoom, ChatMessage
rooms = list(ChatRoom.objects.all())
emit('three_rooms', len(rooms) >= 3)
b = body(c.get('/chat/'))
emit('chat_list_200', c.get('/chat/').status_code == 200)
emit('chat_list_links', all(room.name in b for room in rooms))
emit('chat_list_three_links', b.count('/chat/') >= 3)
r = c.get('/chat/%d/' % rooms[0].pk)
emit('room_anon_redirect', r.status_code in (301, 302))
c.login(username='alice', password='secret123')
b = body(c.get('/chat/%d/' % rooms[0].pk))
emit('room_200', c.get('/chat/%d/' % rooms[0].pk).status_code == 200)
emit('room_name', rooms[0].name in b)
emit('room_websocket', '/ws/chat/' in b)
emit('room_jquery', 'jquery' in b.lower())
emit('room_scroll', 'overflow-y' in b and 'height' in b)

# --- ex02/03/04 : consommateur WebSocket
from channels.routing import URLRouter
from channels.testing import WebsocketCommunicator
from chat import routing
from chat.consumers import CONNECTED_USERS


async def drain(comm, count, timeout=5):
    msgs = []
    for _ in range(count):
        msgs.append(await comm.receive_from(timeout=timeout))
    return msgs


async def ws_scenario(room, alice, bob):
    results = {}
    app = URLRouter(routing.websocket_urlpatterns)

    a = WebsocketCommunicator(app, '/ws/chat/%d/' % room.pk)
    a.scope['user'] = alice
    ok, _ = await a.connect()
    results['a_connect'] = ok
    await drain(a, 3)

    bcomm = WebsocketCommunicator(app, '/ws/chat/%d/' % room.pk)
    bcomm.scope['user'] = bob
    ok, _ = await bcomm.connect()
    results['b_connect'] = ok

    # Alice recoit le join de Bob puis la liste des utilisateurs
    raw = await a.receive_from(timeout=5)
    join_bob = json.loads(raw)
    raw2 = await a.receive_from(timeout=5)
    users = json.loads(raw2)
    results['join_bob'] = join_bob.get('message') == 'bob has joined the chat'
    results['user_list_two'] = sorted(users.get('users', [])) == ['alice', 'bob']

    # Bob recoit le join, sa liste d'utilisateurs puis son historique
    bob_msgs = [json.loads(m) for m in await drain(bcomm, 3)]
    users_b = [m for m in bob_msgs if m.get('type') == 'user_list']
    hist = [m for m in bob_msgs if m.get('type') == 'history']
    results['user_list_self'] = bool(users_b) and 'bob' in users_b[0].get('users', [])
    contents = [m['content'] for m in hist[0].get('messages', [])] if hist else []
    results['history_three'] = contents == ['message 2', 'message 3', 'message 4']

    # Alice envoie un message : tout le monde le recoit
    await a.send_to(text_data=json.dumps({'message': 'bonjour'}))
    m1 = json.loads(await a.receive_from(timeout=5))
    m2 = json.loads(await bcomm.receive_from(timeout=5))
    results['message_broadcast'] = (m1.get('message') == 'bonjour'
                                    and m1.get('username') == 'alice'
                                    and m2.get('message') == 'bonjour')

    # Bob se deconnecte : Alice recoit le leave et la liste mise a jour
    await bcomm.disconnect()
    raw = await a.receive_from(timeout=5)
    leave = json.loads(raw)
    raw = await a.receive_from(timeout=5)
    users_after = json.loads(raw)
    results['leave_bob'] = leave.get('message') == 'bob has left the chat'
    results['user_list_after'] = users_after.get('users') == ['alice']

    await a.disconnect()
    return results


try:
    room = ChatRoom.objects.first()
    ChatMessage.objects.all().delete()
    for i in range(5):
        ChatMessage.objects.create(room=room, username='bob',
                                   content='message %d' % i)
    CONNECTED_USERS.clear()
    alice = User.objects.get(username='alice')
    bob = User.objects.create_user('bob', password='secret123')
    results = asyncio.run(ws_scenario(room, alice, bob))
except Exception as e:
    results = {'error': repr(e)}
for k, v in results.items():
    out.append('ws_' + k + '|' + ('yes' if v is True else ('no' if v is False else str(v))))

print('##')
print('\n'.join(out))
'''


def main() -> int:
    r = Report("d08 - Final")
    tmp = make_tmp()
    probe = tmp / "probe.py"
    probe.write_text(PROBE)
    db = tmp / "db.sqlite3"
    rc, out, err = run_py(probe, [str(PROJECT), str(db)], timeout=300)
    if rc != 0:
        r.fail("d09: probe", err[-900:])
        return r.summary()
    bodytxt = out.split("##\n", 1)[1] if "##\n" in out else ""
    res = {}
    for line in bodytxt.strip().splitlines():
        if "|" in line:
            k, _, v = line.partition("|")
            res[k] = v

    def val(k):
        return res.get(k, "<absent>")

    r.equals("ex00: page account 200", val("account_200"), "yes")
    r.equals("ex00: formulaire login anonyme", val("account_form_anon"), "yes")
    r.equals("ex00: bloc connecte masque anonyme", val("account_hidden_anon"), "yes")
    r.equals("ex00: jQuery present", val("account_jquery"), "yes")
    r.equals("ex00: login AJAX invalide", val("ajax_login_ko"), "yes")
    r.equals("ex00: login AJAX valide", val("ajax_login_ok"), "yes")
    r.equals("ex00: page connectee", val("account_logged"), "yes")
    r.equals("ex00: formulaire masque une fois connecte",
             val("account_logged_form_hidden"), "yes")
    r.equals("ex00: logout AJAX", val("ajax_logout_ok"), "yes")
    r.equals("ex00: retour anonyme apres logout", val("account_back_anon"), "yes")

    r.equals("ex01: trois chatrooms", val("three_rooms"), "yes")
    r.equals("ex01: liste des rooms 200", val("chat_list_200"), "yes")
    r.equals("ex01: noms des rooms affiches", val("chat_list_links"), "yes")
    r.equals("ex01: trois liens", val("chat_list_three_links"), "yes")
    r.equals("ex01: room anonyme redirigee", val("room_anon_redirect"), "yes")
    r.equals("ex01: room connecte 200", val("room_200"), "yes")
    r.equals("ex01: nom du chat affiche", val("room_name"), "yes")
    r.equals("ex01: URL websocket", val("room_websocket"), "yes")
    r.equals("ex01: jQuery", val("room_jquery"), "yes")
    r.equals("ex04: conteneur a defilement", val("room_scroll"), "yes")

    r.equals("ex01: connexion websocket", val("ws_a_connect"), "yes")
    r.equals("ex01: deuxieme utilisateur connecte", val("ws_b_connect"), "yes")
    r.equals("ex01: message de jointure", val("ws_join_bob"), "yes")
    r.equals("ex03: liste des utilisateurs a jour", val("ws_user_list_two"), "yes")
    r.equals("ex03: liste affichee a la connexion", val("ws_user_list_self"), "yes")
    r.equals("ex02: historique de 3 messages", val("ws_history_three"), "yes")
    r.equals("ex01: message diffuse a tous", val("ws_message_broadcast"), "yes")
    r.equals("ex03: message de depart", val("ws_leave_bob"), "yes")
    r.equals("ex03: liste apres depart", val("ws_user_list_after"), "yes")

    return r.summary()


if __name__ == "__main__":
    raise SystemExit(main())
