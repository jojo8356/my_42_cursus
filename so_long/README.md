*This project has been created as part of the 42 curriculum by jojo8356.*

# so_long

A small 2D game: the player collects every collectible on the map, then reaches
the exit. Written in C with the miniLibX, in accordance with the Norm.

`Français` — see [README.fr.md](README.fr.md).

## Contents

- [The subject](#the-subject)
- [What the program does](#what-the-program-does)
- [Build](#build)
- [Running](#running)
- [Map format](#map-format)
- [Bonuses](#bonuses)
- [Tests](#tests)
- [Design notes](#design-notes)
- [Project structure](#project-structure)
- [Resources](#resources)

## The subject

The official subject is shipped in [`docs/`](docs/):

| File | What it is |
|---|---|
| [`docs/en.subject.pdf`](docs/en.subject.pdf) | the official PDF, as distributed by 42 |
| [`docs/en.subject.md`](docs/en.subject.md) | English transcription, for searching and quoting |
| [`docs/fr.subject.md`](docs/fr.subject.md) | French transcription |

The rules the implementation is held to are, in short:

- the program takes a `.ber` map as its only argument;
- the map uses five symbols: `0` empty, `1` wall, `C` collectible, `E` exit,
  `P` player start;
- it must be rectangular, closed by walls, and contain at least one `C`, exactly
  one `E` and exactly one `P`;
- the player moves with `W`, `A`, `S` and `D`, cannot walk into a wall, must
  collect every `C` before the exit becomes usable, and wins by reaching `E`;
- the current move count is printed in the shell on every move;
- `ESC` and the window's red cross both close the window and quit cleanly;
- any malformed map produces `Error` followed by an explicit message.

Note that the subject explicitly does **not** require checking that a path
exists between the player and the exit — and this implementation does not
enforce it either, because refusing a map the subject calls valid would be a bug.

## What the program does

- Loads and validates the map before touching the graphical library, so an
  invalid map never flashes a window open and closed.
- Draws every tile from a 64×64 XPM, then the actors on top.
- Moves the player on the four axes, refusing walls and refusing the exit while
  collectibles remain.
- Prints the move counter in the shell (`1`, `2`, `3`, … on their own lines).
- Switches the exit to its lit sprite once the last collectible is taken, as the
  visual cue that the level can be left.
- Frees everything on exit: the map, the images, the window and the display.

## Build

```sh
make            # mandatory binary: ./so_long
make bonus      # bonus binary:     ./so_long_bonus
make re         # fclean + all
make clean      # objects
make fclean     # objects + binaries
```

The miniLibX is expected as a system library (`-lmlx -lXext -lX11 -lm`), which
is how it is installed on the 42 clusters. To build it from sources instead,
point the `MLX_DIR` variable at a checkout and add the include path:

```sh
make MLX_INC="-I mlx" MLX_LIB="mlx/libmlx.a -lXext -lX11 -lm"
```

Building the graphical binary requires X11 development headers
(`libx11-dev`, `libxext-dev`) — see [Tests](#tests) for what happens without
them.

## Running

```sh
./so_long maps/valid/basic.ber
./so_long_bonus maps/valid/enemies.ber
```

| Key | Action |
|---|---|
| `W` | up |
| `A` | left |
| `S` | down |
| `D` | right |
| `ESC` | quit |

A sample map is provided in [`maps/`](maps/).

## Map format

```
1111111
1P00C01
1011011
1000CE1
1111111
```

| Symbol | Meaning |
|---|---|
| `0` | empty floor |
| `1` | wall |
| `C` | collectible |
| `E` | exit (exactly one) |
| `P` | player start (exactly one) |
| `X` | enemy — **bonus only** |

The mandatory binary rejects `X`: it is not one of the five symbols of chapter
IV. The bonus binary accepts it, and that is the only difference in parsing.

Every rejection prints `Error` on the first line and a specific reason on the
second, for example:

```
Error
maps/invalid/ragged.ber: map is not rectangular
```

The message goes to **stdout**: chapter IV does not name a stream, and the
community tester (see below) decides whether a map was rejected by piping the
program through `grep Error`, which reads stdout.

## Bonuses

All three bonuses of chapter V are implemented, in `_bonus.{c,h}` files and
behind the separate `bonus` Makefile rule, as chapter III requires.

**1. Enemy patrols.** Every `X` on the map becomes an enemy that walks back and
forth along its row, reversing at walls. Contact loses the game, in both
directions: the player walking into an enemy, and an enemy walking into the
player.

**2. Sprite animation.** The player has two frames and so does the enemy; the
frame advances on the game loop timer, so the sprites keep moving even while the
player stands still. The loop draws on a timer rather than only on a key press,
which is what makes the animation visible.

**3. On-screen move counter.** The move count is drawn in the top-left corner of
the window with `mlx_string_put`. The shell counter stays as well, because it is
a mandatory requirement — the bonus is an addition, not a replacement.

## Tests

```sh
bash run_tests.sh              # build, then every layer
bash run_tests.sh game         # one layer only
bash run_tests.sh --no-build   # reuse the previous binaries
```

The suite has six layers, 106 checks in total, plus the 44 checks of a
third-party tester:

| Layer | Checks | What it covers |
|---|---:|---|
| `parse` | 38 | the ten map rules of chapter IV, plus the 41 corpus maps |
| `game` | 16 | movements, walls, exit gating, win, `ESC`, rendering, hooks |
| `bonus` | 18 | enemies, animation, on-screen counter, rules still enforced |
| `memory` | 12 | ASan + UBSan over every map: no leak, no crash |
| `compliance` | 22 | Norm rules, Makefile, assets, subject, READMEs |
| `community` | 44 | `augustobecker/so_long_tester`, run unmodified |

### The headless problem

A graphical project is awkward to test on a machine with no display, and this is
exactly the situation in CI. The suite solves it without modifying a single line
of the game:

- [`tests/shim/mlx.h`](tests/shim/mlx.h) and
  [`tests/shim/mlx_shim.c`](tests/shim/mlx_shim.c) implement the miniLibX API
  faithfully — same prototypes, same semantics — and log every call to
  `$MLX_SHIM_LOG` instead of drawing.
- `tests/build.sh` compiles the **real** sources against that shim, with the same
  `-Wall -Wextra -Werror`, and reads its source list from the `Makefile` so the
  test build can never drift from the real one.
- Two environment variables turn the interactive loop into a scripted run:
  `MLX_SHIM_FRAMES` (how many frames to run) and `MLX_SHIM_KEYS` (which keys to
  feed, one per frame).

That makes it possible to play a complete winning game, lose to an enemy, and
assert on which sprite was blitted where — none of which a screenshot would tell
you reliably. What it cannot do is verify the actual pixels: the suite proves the
right image went to the right tile, not that the tile looks good.

On a machine with X11, `make && make bonus` builds the real binaries, and this
suite exercises the same code with a notebook instead of a screen.

### The third-party tester

[`augustobecker/so_long_tester`](https://github.com/augustobecker/so_long_tester)
is kept verbatim in [`tests/community/augustobecker/`](tests/community/augustobecker/),
with its 13 check scripts and its 41 maps. `tests/cases/60-community.sh` stages
it in a temporary directory (its scripts expect the binary one level above
`tests/`) and runs all thirteen checks: **44 [OK], 0 [KO]**.

Two environment gaps are worked around, and both are announced by the runner:

- `valgrind` is not installable here, so `tests/tools/valgrind` is a stand-in
  that runs the command and skips valgrind's options. It does **not** check
  memory. That job is done for real by the `memory` layer, which runs
  AddressSanitizer and UBSan over the same 41-map corpus.
- The binary is the headless build, because there is no X server.

### Test-design rule

Every expected move count and every expected rejection in the suite is derived
from [`tests/tools/reference_model.py`](tests/tools/reference_model.py), an
independent implementation of chapter IV's rules. Change an expectation by
running that model, never by adjusting the number until the test goes green.

## Design notes

**The grid is never mutated.** The player's position lives in the game state,
not in the map. The alternative — writing `'0'` over the tile the player leaves —
works for collectibles but destroys the exit tile the first time the player steps
on it, which is the classic `so_long` bug. Keeping the grid pristine means the
exit is still an `E` at the end, and drawing is a pure function of game state.

**The bonus is not a fork.** The mandatory and bonus binaries share their parsing,
validation, hooks and cleanup; the differences are behind three small files
(`bonus_charset`, `bonus_frame`, `enemy_bonus`). A bonus that reimplements the
game is a bonus with its own bugs, and chapter V only grades the bonus if the
mandatory part is perfect.

**Validation happens before the window opens.** A rejected map never creates an
X connection, which the suite asserts directly: an invalid map must produce no
`mlx_init` in the shim log.

## Project structure

```
so_long/
├── assets/          nine 64×64 XPM sprites, two frames per animated entity
├── docs/            the subject: PDF plus English and French transcriptions
├── include/         so_long.h
├── libft/           vendored libft, built by its own Makefile
├── maps/            sample maps
├── src/             mandatory sources
│   └── bonus/       the three bonuses, in _bonus.c files
├── tests/
│   ├── build.sh          headless build of both binaries
│   ├── cases/            the six test layers
│   ├── community/        augustobecker/so_long_tester, verbatim
│   ├── shim/             the miniLibX stand-in
│   └── tools/            reference model, valgrind stand-in
├── Makefile
└── run_tests.sh
```

## Resources

- [42Paris/minilibx-linux](https://github.com/42Paris/minilibx-linux) — the
  miniLibX sources, and the reference for its API.
- [augustobecker/so_long_tester](https://github.com/augustobecker/so_long_tester)
  — the community tester used in the `community` layer.
- The sprites in `assets/` were generated for this project (flat pixel art,
  64×64), not taken from a third party, so there is no attribution to carry.

### AI usage

AI assistance was used to write this project and its test suite. Concretely: the
source layout and the test layers were drafted with an AI assistant, then
verified by running them — every count reported above comes from an actual run of
`bash run_tests.sh`, not from the assistant's description. The subject itself was
transcribed from the official PDF. Design decisions, and any mistakes that
remain, are the author's.
