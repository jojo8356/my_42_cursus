*This activity has been created as part of the 42 curriculum by jojo8356.*

# A-Maze-ing

## Description

A-Maze-ing is a maze generator, solver and visualizer written in Python.
It reads a `KEY=VALUE` configuration file, generates a random maze, writes
it to an output file using a hexadecimal wall representation, computes the
shortest path between the entry and the exit, and displays the maze
interactively in the terminal.

The maze generation logic lives in a reusable package, `mazegen`, that is
independent of the configuration file, the terminal and the command line,
and can be installed with pip and imported in a future project.

Every sufficiently large maze contains a visible **"42"** drawn with fully
closed cells. A **perfect maze** (`PERFECT=True`) has exactly one path
between any two cells (no loop); a **Pac-Man board** (`PERFECT=False`)
offers at least two independent routes, open corners and centre, and few
dead-ends.

## Instructions

### Configuration file

The configuration file uses one `KEY=VALUE` pair per line. Lines starting
with `#` are comments. The following keys are supported:

| Key | Description | Example |
| --- | --- | --- |
| `WIDTH` | Number of columns (cells). | `WIDTH=20` |
| `HEIGHT` | Number of rows (cells). | `HEIGHT=15` |
| `ENTRY` | Entry coordinates `x,y`. | `ENTRY=0,0` |
| `EXIT` | Exit coordinates `x,y`. | `EXIT=19,14` |
| `OUTPUT_FILE` | Path of the hexadecimal output file. | `OUTPUT_FILE=maze.txt` |
| `PERFECT` | `True` for a perfect maze, `False` for a Pac-Man board. | `PERFECT=True` |
| `SEED` | Optional integer for reproducible generation. | `SEED=42` |
| `ALGORITHM` | `dfs` (default) or `prim`. | `ALGORITHM=dfs` |

`WIDTH`, `HEIGHT`, `ENTRY`, `EXIT`, `OUTPUT_FILE` and `PERFECT` are
mandatory.

### Run

```bash
python3 a_maze_ing.py config.txt
```

The program writes the maze to `OUTPUT_FILE` and opens an interactive menu
to regenerate the maze, toggle the shortest path, rotate the wall colours
or quit.

### Output file format

Each cell is written as one uppercase hexadecimal digit encoding its four
walls as bits:

| Bit | Direction |
| --- | --- |
| 0 (LSB) | North |
| 1 | East |
| 2 | South |
| 3 | West |

A set bit means the wall is closed. Cells are stored row by row, one row
per line. After an empty line, three lines follow: the entry coordinates,
the exit coordinates, and the shortest path as `N`, `E`, `S`, `W` letters.

### Makefile

```bash
make lint      # flake8 and mypy
make run       # run the main script
make debug     # run under pdb
make clean     # remove caches
make install   # install dependencies
```

## Reusing the mazegen package

The reusable part is the `mazegen` package, installable with pip:

```bash
pip install mazegen-1.0.0-py3-none-any.whl
```

```python
from mazegen import MazeGenerator

# Instantiate with custom parameters (size, seed, mode).
generator = MazeGenerator(15, 11, (0, 0), (14, 10),
                          perfect=True, seed=42)
generator.generate()

# Access the generated structure and at least a solution.
print(generator.walls)   # grid of wall values
print(generator.solve()) # shortest path as N/E/S/W letters
```

## Algorithm

The maze is generated with a **depth-first search** (recursive
backtracker): starting from the entry, a wall is knocked down towards an
unvisited neighbour until every cell has been visited, producing a spanning
tree. A randomized **Prim** algorithm is also available. The shortest path
is found with a **breadth-first search**.

### Why this algorithm

The depth-first search is simple, fast and naturally produces a long,
twisty corridor, which gives a good-looking maze. It also composes cleanly
with the "42" pattern: blocked cells are simply excluded from the walk.
Breadth-first search guarantees the shortest solution, which the subject
requires.

## Team and project management

- **Roles**: one developer handled the reusable `mazegen` core (model,
  generation, validation, solving), the other the CLI front-end (config
  parsing, output writing, rendering and menu).
- **Planning**: the project was planned in slices (parser, generation,
  serialization, display, packaging) and reviewed slice by slice.
- **What worked well / could be improved**: separating the core from the
  UI made each part testable independently; the braiding pass interacting
  with the 3x3 rule could have been planned earlier.
- **Tools**: Python 3.10+, git, flake8, mypy, pytest and pip for the build.

## Resources

The standard library was the main resource (random, collections.deque,
pathlib, typing). The subject PDF defined the maze constraints and the
required output format.

### Use of AI

AI tools were used to review the code and to help write the test suite and
the independent maze analyzer that re-checks the output file (wall
coherence, connectivity, 3x3 areas, loops, dead-ends and path optimality),
without reusing the project's own validator.
