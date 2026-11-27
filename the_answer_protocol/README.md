# 🔐 The Answer Protocol

A complete, stdlib-only implementation of **The Answer Protocol (42TAP)**: an
authoritative multiplayer text adventure over TCP, a terminal client, and a
self-contained browser GUI. Players share one live Arrakis world, see presence
and combat events, collect items, complete quests, chat and form groups.

The project is designed for Python 3.10+. **The game server, TCP clients, GUI
bridge, world validator and demo have no third-party runtime dependencies.**
`pytest` is an optional development dependency used by the test suite.

## At a glance

| Component | Implementation |
|---|---|
| Wire protocol | UTF-8, LF-delimited TCP, compact JSON, async events |
| Server | `socket` + `threading`; one reader and one writer worker per connection |
| Shared state | Serialized mutations under an `RLock`; authoritative inventory, NPC, quest and group state |
| CLI client | Raw command pass-through; async reader and terminal redraw while typing |
| GUI client | Inline HTML/CSS/vanilla JS served by stdlib HTTP + WebSocket; TCP is proxied server-side |
| World | Validated JSON data (`data/world.json`) |
| Security/reliability | 1024-byte line cap, UTF-8 validation, bounded output queues, flood control, per-IP connection throttling, graceful cleanup |
| Dependencies | Python standard library at runtime; pytest/coverage only for development |

## Quick start

```sh
# Optional: install test and coverage tools. Production code needs no install step.
python3 -m venv .venv
. .venv/bin/activate
make install
make test

# Terminal 1: authoritative TCP server
make run-server

# Terminal 2: terminal client
make run-client

# Or, in another terminal: browser client
make run-client-gui
# Open http://127.0.0.1:8080
```

The CLI starts at `127.0.0.1:4242`; the GUI is served on `0.0.0.0:8080` and
connects to the TCP server at `127.0.0.1:4242`. For another port or host:

```sh
python3 tap.py server --host 127.0.0.1 --port 4243 --world data/world.json
python3 tap.py client --host 127.0.0.1 --port 4243
python3 tap.py gui --host 0.0.0.0 --port 8080 --tap-host 127.0.0.1 --tap-port 4243
```

At the prompt, use `CONNECT <callsign>` (1–20 ASCII letters, digits, `_` or
`-`) and then `HELP`. The GUI asks for a callsign after opening the WebSocket.

### Useful commands

```sh
python3 tap.py check-world                 # validate references and show world counts
python3 tap.py check-world --json          # machine-readable validation report
python3 tap.py demo                        # two TCP clients play a scripted scenario
make lint                                  # compile-check project Python files
make coverage                              # tests + 100% statement/branch report
make clean                                 # remove Python/test caches
```

`demo` starts an ephemeral server and drives the real protocol through two
separate TCP sockets; it does not mutate or require an external server.

## Project layout

```text
tap.py                       CLI: server / client / gui / check-world / demo
src/protocol.py              RFC 42TAP parser, error table, JSON and event builders
src/world.py                 JSON loader, canonical IDs, referential validation
src/state.py                 Players, groups, quests, combat, inventory, dispatcher
src/server.py                TCP listener, per-client queues, broadcasts, timers
src/client_cli.py            Non-blocking terminal client
src/client_gui.py            Inline SPA + HTTP/WebSocket-to-TCP bridge
src/logging_json.py          Thread-safe JSON-lines logger
data/world.json              Arrakis world data
tests/                       Unit, socket, client, concurrency and end-to-end tests
subject.txt                  Full project specification and implementation contract
```

Imports in `src/` are intentionally flat so the modules work when launched from
`tap.py` as scripts and when `PYTHONPATH=src` is used by tests.

## Protocol contract

### Framing and lifecycle

* TCP stream of UTF-8 messages, each terminated by one LF (`0x0A`). The server
  removes a single trailing CR on input to interoperate with CRLF senders.
* The greeting is `OK hello proto=1` as soon as a TCP connection is accepted.
* Before `CONNECT`, only `CONNECT` and `QUIT` are allowed. Other commands receive
  `ERR 400 NOT_AUTHENTICATED`.
* A non-empty client line gets exactly one `OK` or `ERR` reply. Asynchronous
  `EVT`/chat messages can appear between replies; clients must not assume that
  the next line is always the response.
* Verb and scope tokens are case-insensitive. Free-form arguments preserve case
  and are parsed as one rest-of-line value, so item names and multi-word chat
  survive intact. Blank lines are silent.
* UTF-8 command lines are capped at **1024 bytes**. An invalid UTF-8 line gets
  `ERR 400 BAD_REQUEST`; an overlong line gets one `ERR 413 LINE_TOO_LONG` and
  is discarded through its LF boundary before parsing resumes.
* JSON is compact (no embedded line breaks); empty JSON collections are `[]`,
  never `null`. Clients should inspect both `code` and `symbol`: RFC 42TAP shares
  numeric code 404 across several distinct errors.

### Commands

| Command | Successful reply / behavior |
|---|---|
| `CONNECT <username>` | `OK Connected`; duplicate names return `201 NAME_IN_USE` |
| `LOOK` | Flat JSON room object: `id`, `name`, `description`, `exits`, `players`, `items`, `npcs`, optional `can_sleep` |
| `MOVE <direction>` | `OK room=<loc.id>`; `301 NO_EXIT` or `416 IN_COMBAT` |
| `WHO` | `OK players=<count>` |
| `CHAT GLOBAL\|ROOM\|GROUP <message>` | `OK`; chat event is sent to recipients, including the sender |
| `SAY <message>` | Room-chat shorthand |
| `TALK <npc>` | `OK {"npc":...,"dialogue":...}`; dialogue cycles |
| `TAKE <item>` / `DROP <item>` | `OK taken=<item.id>` / `OK dropped=<item.id>`; room event to other players |
| `INVENTORY` | JSON array of item IDs |
| `EXAMINE <item>` | JSON item details; searches the room, then inventory |
| `STATUS` | JSON `{hp,max_hp,gold,status}` (`status` is `alive` or `dead`) |
| `ATTACK <npc>` | JSON combat round; player strikes first and the enemy counters if it survives |
| `DEFEND` / `FLEE` | Combat-only bonus actions; errors with `415 NOT_IN_COMBAT` otherwise |
| `SLEEP` | Restores full HP only in a sleep room (`410 CANNOT_SLEEP_HERE` elsewhere) |
| `QUEST <npc>` / `QUESTS` | Accept, progress and turn in quests; JSON state/progress |
| `ABANDON_QUEST <id>` | Abandon an active quest without consuming its items |
| `GROUP CREATE <name>` | Create a named group (name omitted: player's callsign) |
| `GROUP INVITE <player>` | Leader-only invitation; sends `EVT GROUP INVITE <inviter>` |
| `GROUP JOIN <group-or-inviter>` | Accept a pending invite by group ID or inviter name |
| `GROUP LEAVE` / `GROUP KICK <player>` | Leave or leader-remove a member; first remaining member becomes leader |
| `GROUP DISBAND` / `GROUP INFO` | Leader dissolves group / JSON group state |
| `HELP` | `OK commands=<verbs>` |
| `QUIT` | `OK bye`, then orderly cleanup |

NPCs and items match by exact ID, ID suffix (`npc.guard` → `guard`), exact display
name, name prefix, then substring (the first matching world entry wins if a
substring is ambiguous).

### Errors

The RFC entries below retain their numeric codes and symbols. Extensions are
explicit implementation decisions (documented in `subject.txt`). All lines are
rendered as `ERR <code> <SYMBOL>`.

| Code | Symbol(s) | Meaning |
|---:|---|---|
| 201 | `NAME_IN_USE` | Callsign already connected |
| 301 | `NO_EXIT` | Direction is not an exit from the current room |
| 401 | `NOT_IN_GROUP` | Group-only command without a group |
| 402 | `ALREADY_IN_GROUP` | Already in a group or already invited |
| 404 | `ITEM_NOT_FOUND`, `ITEM_NOT_IN_INVENTORY`, `NPC_NOT_FOUND`, `PLAYER_NOT_FOUND`, `GROUP_NOT_FOUND`, `PLAYER_NOT_IN_GROUP`, `QUEST_NOT_ACTIVE` | The named entity/state is not present |
| 405 | `NPC_NOT_HOSTILE` | Friendly NPC cannot be attacked |
| 406 | `NO_QUEST_AVAILABLE` | No currently available quest at this giver |
| 202 | `INVALID_USER_NAME` | Invalid callsign |
| 211 / 212 | `GROUP_NAME_IN_USE` / `INVALID_GROUP_NAME` | Group name collision / invalid format |
| 400 | `NOT_AUTHENTICATED`, `BAD_REQUEST`, `UNKNOWN_COMMAND`, `MISSING_ARGUMENT` | Authentication, parse, command or required-argument error |
| 403 | `NOT_GROUP_LEADER` | Leader-only action |
| 407 | `CANNOT_INVITE_SELF`, `CANNOT_KICK_SELF`, `NOT_INVITED` | Invitation/member rules |
| 408 | `QUEST_NOT_COMPLETE` | Quest objective not complete |
| 409 | `PLAYER_DEAD`, `ALREADY_CONNECTED` | Player state / duplicate socket authentication |
| 410 | `CANNOT_SLEEP_HERE` | Not in a safe room |
| 413 | `LINE_TOO_LONG` | UTF-8 line exceeded 1024 bytes |
| 415 / 416 | `NOT_IN_COMBAT` / `IN_COMBAT` | Combat-only action / movement during combat |
| 900 / 901 / 902 | `CONNECTION_FAILED` / `SEND_FAILED` / `FLOODING` | Fatal connection, send or abuse error |

`900+` is the fatal split. `404` is intentionally non-unique by symbol; code
alone is never sufficient to identify the failure.

### Asynchronous events

* `EVT ROOM PRESENCE ENTER|LEAVE <player>`
* `EVT ROOM ITEM TAKE|DROP <player> <item-id>`
* `EVT ROOM COMBAT ...`, `EVT ROOM NPC RESPAWN <npc-id>`,
  `EVT ROOM ITEM RESPAWN <item-id>`, and sleep/respawn events
* `EVT GROUP INVITE|JOIN|LEAVE|LEADER|KICK|DISBAND ...`
* `EVT STATS players=<count>`
* `(GLOBAL|ROOM|GROUP) <player>: <message>`

Room ENTER/LEAVE events exclude the actor (the actor has the corresponding
command reply); chat includes the sender. A single writer thread serializes every
outgoing line for each client, so pushed events and replies never interleave at
the byte level.

## Server and concurrency model

* A single listener accepts up to 128 concurrent sockets; every socket has a
  buffered reader and its own bounded (512-line) outbound queue/writer.
* `GameState` protects connect, dispatch, disconnect and timer ticks with one
  re-entrant lock. Two simultaneous TAKE commands cannot both remove the same
  ground item.
* Slow output never blocks a broadcast. A client whose output queue fills is
  dropped; a failed `sendall` cleans only that connection and player.
* Per-client flood control uses a one-second sliding window: command 11 warns;
  command 21 returns `902 FLOODING` and closes the session. CONNECT counts toward
  the window. A separate per-IP guard rejects over 10 rapid connections within
  10 seconds; 128 active connections is the global cap.
* EOF, QUIT, flood kicks and send failures all run the same idempotent cleanup:
  group membership and invites are cleaned, inventory is dropped in the current
  room, presence/stats events are emitted, and sockets are closed.
* SIGINT/SIGTERM stops the accept loop, stops timer workers and removes every
  connected player. Structured logs are emitted as one JSON object per line to
  stderr (time, level, msg, type, player, ip, command/response and error context).

## World and game rules

`World.load()` canonicalizes bare IDs to `loc.`, `item.`, `npc.` and `quest.`
IDs (`room.` is accepted as an alias for `loc.`). Startup fails with a
`WorldError` if the JSON structure or references are invalid: exits, placed
items/NPCs, quest giver/target/reward/recipient/prerequisite, sleep room and start
room are checked before the listener binds.

The packaged Arrakis world has 9 rooms, 8 item definitions, 7 NPCs and 3 quests:

1. **Spice harvest** — bring Chani two `raw_melange` items from the market.
2. **Harkonnen raid** — defeat the hostile scout in the smugglers' den.
3. **Fremen trust** — recover the Fremen token in Sietch Tabr; this quest is
   locked until the spice-harvest quest is completed.

Quest progress is tracked as live inventory counts or kill progress. The player
must explicitly return and ask the giver (or deliver recipient) to turn in an
objective. Fetch/deliver objectives are consumed only at completion; rewards are
added to inventory. Abandoned quests do not consume carried objective items.

Combat damage is base 10 plus the strongest carried damage bonus; the strongest
carried armor reduces an enemy's counterattack. Defeated enemies return at max HP
after 30 seconds; taken items respawn after 30 seconds. A dead player is hidden
from room presence for 3 seconds, then returns to the start room at 50 HP while
retaining inventory. Sleeping restores full HP in the configured safe room.

## Browser GUI

The GUI is a responsive single-page application with live room/exit controls,
player HP and group state, inventory, quest log, nearby interaction buttons,
chat and a raw-command console. HTML, CSS and JavaScript are inline: there is no
CDN, build step or browser-side third-party asset.

The browser opens a same-origin relative WebSocket at `/ws`. The GUI HTTP process
opens the TCP socket to the TAP server server-side and relays masked WebSocket
text frames to newline-framed TAP commands. Browser code never calls
`localhost`/`127.0.0.1`; this also makes the GUI work behind the Arena preview
proxy. The bridge enforces RFC 6455 client masking, reserved bits, canonical
extended lengths, fragmentation order, control-frame rules, UTF-8 validity and
message-size limits. It mirrors valid close payloads and uses standard close
codes (`1002` protocol error, `1003` unsupported data, `1007` invalid UTF-8,
`1009` message too large); ping payloads are returned byte-for-byte in a pong.

## Tests

The test suite includes parser/error-table tests, malformed-world validation,
state-machine scenarios, quest chains and delivery, combat/respawn, all group
roles, flood limits, item races under threads, real TCP fragmentation/coalescing,
multi-client broadcasts, dead-socket isolation, HTTP routes, WebSocket handshake,
close-code, masking, length, control-frame and fragmentation cases, terminal
redraw/input behavior, CLI pass-through, and a full two-player socket scenario.

```sh
make test
make coverage
# Or, without Make:
PYTHONPATH=src python3 -m pytest tests -q
PYTHONPATH=src python3 -m coverage run --branch --source=src -m pytest tests -q
python3 -m coverage report --fail-under=100
```

**Current result: 252 tests passed (252/252); 100% statement and branch coverage
across all seven `src/` modules (2,119 statements and 714 branches).** The
runtime remains stdlib-only; `pytest` and `coverage` are optional development
and test dependencies in `requirements.txt`.
