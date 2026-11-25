# exam_rank_05

Solutions to the **42 School Exam Rank 05** (new 2025 subject), in C and C++98.

## Projects

### Level 0 — C++98

| Project | Description | Files |
|---------|-------------|-------|
| **vect2** | 2D vector class with arithmetic operators | `vect2.hpp`, `vect2.cpp` |
| **bigint** | Arbitrary-precision unsigned integer | `bigint.hpp`, `bigint.cpp` |
| **polyset** | `searchable_array_bag`, `searchable_tree_bag`, `set` | `searchable_*.hpp/cpp`, `set.hpp/cpp` |

### Level 1 — C

| Project | Description | Files |
|---------|-------------|-------|
| **Life** | Conway's Game of Life | `life.c`, `life.h` |
| **BSQ** | Biggest square on a map | `bsq.c`, `bsq.h` |

## Features

### vect2
- Constructors, copy, assignment, destructor (orthodox canonical form)
- `+`, `-`, `*` (scalar), `/` (scalar, bonus), unary `-`
- `+=`, `-=`, `*=`, `/=`
- `++`, `--` (pre/post)
- `[]` (read/write, const & non-const)
- `==`, `!=`
- `<<` stream output

### bigint
- Arbitrary-precision unsigned integer (stored as base-10 digits)
- `+`, `+=`, `++`, `--`
- Digitshift: `<<`, `>>`, `<<=`, `>>=` (base-10, e.g. `42 << 3 == 42000`)
- Comparison: `<`, `<=`, `>`, `>=`, `==`, `!=`
- `<<` stream output (no leading zeros)
- Bonus: `-`, `*`, `/`, `%` (and int overloads)

### polyset
- `searchable_array_bag`: inherits `array_bag` + `searchable_bag`, implements `has()`
- `searchable_tree_bag`: inherits `tree_bag` + `searchable_bag`, implements `has()`
- `set`: wraps a `searchable_bag`, prevents duplicates
- Orthodox canonical form, const correctness
- Bonus: `count()`, `empty()` accessors

### Life
- Simulates Conway's Game of Life
- `./life width height iterations`
- Reads board from stdin via pen commands: `w`/`a`/`s`/`d` (move), `x` (toggle pen)
- Outputs `O` (alive) / ` ` (dead)
- Uses only allowed functions: `atoi`, `read`, `putchar`, `malloc`, `calloc`, `realloc`, `free`

### BSQ
- Finds the biggest square on a map, avoiding obstacles
- `./bsq file1 file2 ...` or reads stdin
- Validates maps, prints `map error` for invalid maps
- DP algorithm for largest square
- Multiple maps with blank line separators
- Uses only allowed functions: `malloc`, `calloc`, `realloc`, `free`, `fopen`, `fclose`, `getline`, `fscanf`, `fputs`, `fprintf`

## Build

```bash
make          # build all projects
```

Or individually:
```bash
make -C level-0/vect2
make -C level-0/bigint
make -C level-0/polyset
make -C level-1/Life
make -C level-1/BSQ
```

## Test

```bash
python3 tests/run_tests.py
```

The suite builds all projects and runs unit tests (C++) + integration tests
(Python) covering every feature and bonus.

## Structure

```
exam_rank_05/
├── Makefile                 # build all
├── README.md
├── subject.md               # full subjects
├── level-0/
│   ├── vect2/               # vect2.hpp, vect2.cpp, main.cpp, subject.txt
│   ├── bigint/              # bigint.hpp, bigint.cpp, main.cpp, subject.txt
│   └── polyset/
│       ├── given-code/      # provided base classes + main
│       ├── searchable_array_bag.hpp/cpp
│       ├── searchable_tree_bag.hpp/cpp
│       ├── set.hpp/cpp
│       └── subject.txt
├── level-1/
│   ├── Life/                # life.c, life.h, subject.txt
│   └── BSQ/                 # bsq.c, bsq.h, subject.txt
└── tests/                   # run_tests.py, test_*.cpp, test_*.py
```
