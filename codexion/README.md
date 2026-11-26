*This project has been created as part of the 42 curriculum by jojo8356*

# Codexion

## Description

**Codexion** is a concurrency simulation built on the classic *dining
philosophers* problem. `number_of_coders` coder threads sit around a circular
table of shared USB dongles. Compiling requires holding both neighboring
dongles at once (the coder's left and right dongle), so two adjacent coders can
never compile at the same time, but non-adjacent ones can. Each coder repeats
a cycle of four states: **waiting** for its two dongles, **compiling**,
**debugging** and **refactoring**.

A dedicated **monitor** thread watches every coder's deadline and stops the
whole simulation as soon as either:

- a coder **burns out** (fails to start a new compile within
  `time_to_burnout` milliseconds of its last one), or
- every coder has reached `number_of_compiles_required` successful compiles.

The goal is to practice POSIX threads synchronization (mutexes, condition
variables) on a non-trivial shared-resource problem and to reason concretely
about deadlock, Coffman's conditions, starvation, and the practical difference
between two classic scheduling policies.

There are **no global variables**: everything lives in one `t_sim` structure,
allocated in `main` and reached by every thread through the pointer it was
handed at creation.

## Instructions

### Build

```bash
make            # builds ./codexion
make clean      # removes object files, keeps the binary
make fclean     # removes object files and the binary
make re         # full rebuild
```

### Run

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> \
  <time_to_debug> <time_to_refactor> <number_of_compiles_required> \
  <dongle_cooldown> <fifo|edf>
```

| Argument | Meaning | Constraint |
| --- | --- | --- |
| number_of_coders | Number of coders (= number of dongles) | integer > 0 |
| time_to_burnout | Max ms without starting a compile | integer > 0 |
| time_to_compile | Duration of a compile (ms) | integer >= 0 |
| time_to_debug | Duration of debugging (ms) | integer >= 0 |
| time_to_refactor | Duration of refactoring (ms) | integer >= 0 |
| number_of_compiles_required | Compiles each coder must do | integer >= 0 |
| dongle_cooldown | Cooldown after a dongle is released (ms) | integer >= 0 |
| scheduler | Scheduling policy | `fifo` or `edf` |

Examples:

```bash
./codexion 5 800 200 200 200 10 50 fifo
./codexion 4 1000 100 100 100 3 10 edf
./codexion 1 800 200 200 200 10 0 fifo   # single coder -> burnout
```

Invalid arguments are rejected without starting a simulation (non-zero exit
code).

### Logs

Each state change is printed as `timestamp_in_ms X <message>`. Messages:
`has taken a dongle`, `is compiling`, `is debugging`, `is refactoring`,
`burned out`. No line is ever interleaved with another, timestamps are
monotonic, and the burnout message is the last line.

## Resources

- Dijkstra, E. W. — *Dining Philosophers Problem*, the classical formulation.
- Coffman, E. G. et al. — the four necessary conditions for deadlock
  (mutual exclusion, hold and wait, no preemption, circular wait).
- POSIX Threads Programming — `pthread_create`, `pthread_mutex_t`,
  `pthread_cond_t`.
- man pages for `pthread_mutex_init`, `pthread_cond_wait`,
  `pthread_cond_broadcast`, `clock_gettime`, `usleep`.
- Wikipedia — Dining Philosophers problem, Earliest Deadline First scheduling.

## Blocking cases handled

- **Deadlock prevention (Coffman's conditions).** Each coder acquires its two
  dongles in a fixed global order (the dongle with the lower index first),
  which breaks the *circular wait* condition. With a single coder both
  dongles are the same object; the coder can hold it only once and therefore
  can never compile, so it burns out instead of self-deadlocking.
- **Starvation prevention.** Every dongle keeps a **min-heap** priority queue
  of its waiters. Under `fifo` the key is the arrival ticket (strictly first
  come, first served); under `edf` the key is the coder's burnout deadline
  (`last_compile_start + time_to_burnout`), so the most exhausted coder is
  served first. No waiter is passed over indefinitely.
- **Cooldown.** A released dongle stays unavailable for `dongle_cooldown` ms.
  A dedicated monitor broadcasts on dongles that have waiters, so a waiting
  coder is woken when a cooldown simply expires (a time-based event that
  carries no signal of its own).
- **Burnout precision.** The monitor polls every millisecond and stops the
  simulation within a few ms of a coder's deadline, well under the required
  10 ms tolerance.
- **Log serialization.** A single log mutex wraps every `printf`, so two lines
  can never interleave.

## Thread synchronization mechanisms

- **One mutex + one condition variable per dongle** protect `in_use`,
  `released_at` and the wait queue. Coders sleep on the condition variable
  instead of busy-polling and wake correctly whether the trigger was a
  release, a cooldown expiring, or the simulation stopping.
- **A per-coder state mutex** protects `compile_count` and
  `last_compile_start`, which are written by the coder's own thread and read
  concurrently by the monitor.
- **A single shared stop flag** (guarded by its own mutex) signals every
  thread to exit cleanly. It is set once and never toggled back, so a slightly
  late read is harmless. The stop flag is never held simultaneously with a
  dongle lock, avoiding lock-ordering deadlocks.
- **One log mutex** serializes all output across every thread.
- **The heap** (`dongle/heap.c`) is a fully generic binary min-heap that only
  compares a key; `fifo` and `edf` are the only places that decide what that
  key means.
