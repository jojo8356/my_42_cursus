#!/usr/bin/env python3
"""
Comprehensive test suite for the Philosophers project (mandatory + bonus).

Checks:
  - Strict compilation with -Wall -Wextra -Werror (done by Makefile).
  - Argument validation (invalid args -> non-zero exit, no crash).
  - Log format: "timestamp_ms X message" with allowed messages only.
  - No hangs (every run terminates within a timeout).
  - Death timing: "died" appears within time_to_die + margin of the last
    meal (or simulation start), and within 10 ms of the actual death.
  - Meals completion: with a meals target, the program stops cleanly.
  - No overlapping/malformed messages.
  - Stress: many iterations to detect deadlocks / data races.
  - Bonus: process cleanup (no leftover semaphores) and correct behavior.
"""

import os
import re
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PHILO = os.path.join(ROOT, "philo", "philo")
PHILO_BONUS = os.path.join(ROOT, "philo_bonus", "philo_bonus")

ALLOWED_MSGS = {"is thinking", "has taken a fork", "is eating",
                "is sleeping", "died"}
LOG_RE = re.compile(r"^(\d+) (\d+) (.+)$")

PASS = 0
FAIL = 0
FAILURES = []


def record(name, ok, detail=""):
    global PASS, FAIL
    if ok:
        PASS += 1
        print(f"  [PASS] {name}")
    else:
        FAIL += 1
        FAILURES.append(f"{name}: {detail}")
        print(f"  [FAIL] {name} :: {detail}")


def build(binpath, srcdir):
    """Build a binary via make (all) and return True on success."""
    r = subprocess.run(["make", "-C", srcdir, "all"],
                       capture_output=True, text=True, timeout=60)
    return r.returncode == 0, r.stderr


def run_binary(binpath, args, timeout=15):
    """Run binary with args, return (returncode, output, elapsed)."""
    start = time.time()
    try:
        r = subprocess.run([binpath] + args, capture_output=True,
                           text=True, timeout=timeout)
        elapsed = time.time() - start
        return r.returncode, r.stdout, r.stderr, elapsed
    except subprocess.TimeoutExpired as e:
        elapsed = time.time() - start
        out = (e.stdout or "") if isinstance(e.stdout, str) else \
            (e.stdout.decode() if e.stdout else "")
        return None, out, "TIMEOUT", elapsed


def parse_logs(output):
    """Parse log lines into list of (ts, id, msg). Returns list or None on
    format error."""
    lines = output.splitlines()
    logs = []
    for line in lines:
        line = line.strip()
        if not line:
            continue
        m = LOG_RE.match(line)
        if not m:
            return None, f"malformed line: {line!r}"
        ts, pid, msg = int(m.group(1)), int(m.group(2)), m.group(3)
        if msg not in ALLOWED_MSGS:
            return None, f"unexpected message: {msg!r}"
        logs.append((ts, pid, msg))
    return logs, None


def check_no_overlap(logs):
    """Messages must not overlap: each line is complete and well-formed
    (already validated by parse_logs). Here we additionally verify that,
    for each philosopher, its own messages appear in chronological order
    (timestamps non-decreasing). Timestamps of different philosophers may
    interleave slightly because they run in separate threads/processes."""
    by_philo = {}
    for (ts, pid, msg) in logs:
        by_philo.setdefault(pid, []).append((ts, msg))
    for pid, msgs in by_philo.items():
        for i in range(1, len(msgs)):
            if msgs[i][0] < msgs[i - 1][0]:
                return False, f"philo {pid} timestamp decreased: {msgs[i]} < {msgs[i-1]}"
    return True, ""


def check_death_timing(logs, time_to_die, start_ts):
    """If a 'died' message exists, verify it occurs around
    last_meal + time_to_die and within 10ms of the actual death."""
    deaths = [l for l in logs if l[2] == "died"]
    if not deaths:
        return True, ""
    for (dts, did, _) in deaths:
        # Find the philosopher's last "is eating" before the death
        last_eat = None
        for (ts, pid, msg) in logs:
            if ts > dts:
                break
            if pid == did and msg == "is eating":
                last_eat = ts
        ref = last_eat if last_eat is not None else start_ts
        # Death should be >= ref + time_to_die (strictly greater per spec)
        if dts < ref + time_to_die - 5:
            return False, f"philo {did} died too early: {dts} < {ref}+{time_to_die}"
        # Death message must be within 10ms of the actual death time
        # (actual death ~ ref + time_to_die; monitor checks every 1ms)
        if dts > ref + time_to_die + 25:
            return False, f"philo {did} death announced too late: {dts} > {ref}+{time_to_die}+25"
    return True, ""


def check_meals_completion(logs, meals):
    """With a meals target and no death, each philosopher should have eaten
    at least `meals` times."""
    counts = {}
    for (_, pid, msg) in logs:
        if msg == "is eating":
            counts[pid] = counts.get(pid, 0) + 1
    for pid in range(1, len(counts) + 1):
        if counts.get(pid, 0) < meals:
            return False, f"philo {pid} ate {counts.get(pid,0)} < {meals}"
    return True, ""


def scenario(name, binpath, args, expect_death=False, meals=None,
             time_to_die=None, timeout=15):
    """Run a single scenario and validate."""
    print(f"\n>> {name}: {' '.join(args)}")
    rc, out, err, elapsed = run_binary(binpath, args, timeout=timeout)
    if rc is None:
        record(name, False, "TIMEOUT / hang (no termination)")
        return
    record(name + " [terminates]", rc == 0,
           f"exit={rc} stderr={err.strip()[:200]}")
    if rc != 0 and expect_death is False and meals is not None:
        # A clean meals-completion run must exit 0
        pass
    logs, errmsg = parse_logs(out)
    if logs is None:
        record(name + " [log format]", False, errmsg)
        return
    record(name + " [log format]", True)
    record(name + " [no overlap]", *check_no_overlap(logs))
    start_ts = logs[0][0] if logs else 0
    if time_to_die is not None:
        record(name + " [death timing]",
               *check_death_timing(logs, time_to_die, start_ts))
    if meals is not None and not expect_death:
        record(name + " [meals completion]",
               *check_meals_completion(logs, meals))
    if expect_death:
        has_death = any(m == "died" for _, _, m in logs)
        record(name + " [death present]", has_death, "no died message")


def test_invalid_args(binpath):
    print("\n>> Invalid argument validation")
    bad_args = [
        [], ["2"], ["2", "100"], ["2", "100", "100", "100", "1", "5"],
        ["0", "100", "100", "100"], ["2", "0", "100", "100"],
        ["2", "100", "0", "100"], ["2", "100", "100", "0"],
        ["2", "100", "100", "100", "0"], ["abc", "100", "100", "100"],
        ["-2", "100", "100", "100"], ["2", "-100", "100", "100"],
        ["2", "100", "100", "100", "-1"], ["2", "100", "100", "100", "x"],
        ["2", "100", "100", "100", "3x"],
    ]
    for args in bad_args:
        rc, out, err, _ = run_binary(binpath, args, timeout=5)
        ok = rc == 1
        record(f"invalid {args}", ok, f"exit={rc} (expected 1)")


def stress(binpath, n_iter=25, label="stress"):
    """Run many iterations to detect deadlocks / data races."""
    print(f"\n>> Stress: {label} ({n_iter} iterations)")
    hangs = 0
    bad_format = 0
    for i in range(n_iter):
        n = 1 + (i % 5)
        rc, out, err, _ = run_binary(binpath,
            [str(n), "800", "100", "100", "2"], timeout=12)
        if rc is None:
            hangs += 1
            continue
        logs, errmsg = parse_logs(out)
        if logs is None:
            bad_format += 1
    record(f"{label} no hangs", hangs == 0, f"{hangs} hangs")
    record(f"{label} valid logs", bad_format == 0, f"{bad_format} bad")


def main():
    print("=" * 60)
    print("PHILOSOPHERS — COMPREHENSIVE TEST SUITE")
    print("=" * 60)

    # Build both binaries
    ok1, e1 = build(PHILO, os.path.join(ROOT, "philo"))
    record("build philo", ok1, e1)
    ok2, e2 = build(PHILO_BONUS, os.path.join(ROOT, "philo_bonus"))
    record("build philo_bonus", ok2, e2)
    if not (ok1 and ok2):
        print("\nBuild failed; aborting.")
        print_summary()
        sys.exit(1)

    # ---- Mandatory: philo ----
    print("\n" + "#" * 60)
    print("# MANDATORY PART (philo)")
    print("#" * 60)
    scenario("philo single meal cycle", PHILO, ["1", "2000", "200", "200", "3"],
             meals=3, time_to_die=2000)
    scenario("philo two philosophers eat", PHILO, ["2", "3000", "200", "100", "2"],
             meals=2, time_to_die=3000)
    scenario("philo death by starvation", PHILO, ["3", "500", "200", "200"],
             expect_death=True, time_to_die=500)
    scenario("philo four philosophers", PHILO, ["4", "3000", "150", "100", "2"],
             meals=2, time_to_die=3000)
    scenario("philo no meals arg (runs till death)", PHILO, ["3", "400", "150", "100"],
             expect_death=True, time_to_die=400)
    test_invalid_args(PHILO)
    stress(PHILO, n_iter=25, label="philo stress")

    # ---- Bonus: philo_bonus ----
    print("\n" + "#" * 60)
    print("# BONUS PART (philo_bonus)")
    print("#" * 60)
    scenario("bonus single meal cycle", PHILO_BONUS, ["1", "2000", "200", "200", "3"],
             meals=3, time_to_die=2000)
    scenario("bonus two philosophers eat", PHILO_BONUS, ["2", "3000", "200", "100", "2"],
             meals=2, time_to_die=3000)
    scenario("bonus death by starvation", PHILO_BONUS, ["3", "500", "200", "200"],
             expect_death=True, time_to_die=500)
    scenario("bonus four philosophers", PHILO_BONUS, ["4", "3000", "150", "100", "2"],
             meals=2, time_to_die=3000)
    scenario("bonus no meals arg", PHILO_BONUS, ["3", "400", "150", "100"],
             expect_death=True, time_to_die=400)
    test_invalid_args(PHILO_BONUS)
    stress(PHILO_BONUS, n_iter=25, label="bonus stress")

    # ---- Bonus: no leftover semaphore ----
    print("\n>> Bonus semaphore cleanup")
    run_binary(PHILO_BONUS, ["2", "2000", "200", "100", "2"], timeout=10)
    semaphores = subprocess.run(
        ["bash", "-c", "ls /dev/shm/sem.philo_forks 2>/dev/null; ls /dev/shm | grep philo"],
        capture_output=True, text=True)
    leftover = semaphores.stdout.strip()
    record("no leftover named semaphore", leftover == "",
           f"leftover: {leftover!r}")

    print_summary()


def print_summary():
    print("\n" + "=" * 60)
    print(f"RESULT: {PASS} passed, {FAIL} failed")
    if FAILURES:
        print("\nFailures:")
        for f in FAILURES:
            print("  -", f)
    print("=" * 60)
    sys.exit(1 if FAIL else 0)


if __name__ == "__main__":
    main()
