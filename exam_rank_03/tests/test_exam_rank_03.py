#!/usr/bin/env python3
"""
Comprehensive test suite for Exam Rank 03 (42 School).

Covers all 8 questions (level 1 mandatory + level 2 bonus):
  filter, ft_scanf, get_next_line, n_queens, permutations, powerset, rip, tsp

For each algorithmic question, the program output is compared against a
reference implementation in Python, so correctness is verified at 100%.
Also checks: strict compilation, argument validation, edge cases, stress.
"""
import os
import re
import subprocess
import sys
import itertools

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

BIN = {
    "filter": os.path.join(ROOT, "filter", "filter"),
    "ft_scanf": os.path.join(ROOT, "ft_scanf", "test_ft_scanf"),
    "gnl": os.path.join(ROOT, "get_next_line", "test_gnl"),
    "n_queens": os.path.join(ROOT, "n_queens", "n_queens"),
    "permutations": os.path.join(ROOT, "permutations", "permutations"),
    "powerset": os.path.join(ROOT, "powerset", "powerset"),
    "rip": os.path.join(ROOT, "rip", "rip"),
    "tsp": os.path.join(ROOT, "tsp", "tsp"),
}

DIRS = ["filter", "ft_scanf", "get_next_line", "n_queens",
        "permutations", "powerset", "rip", "tsp"]

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


def build_all():
    ok = True
    for d in DIRS:
        r = subprocess.run(["make", "-C", os.path.join(ROOT, d), "all"],
                           capture_output=True, text=True, timeout=60)
        if r.returncode != 0:
            ok = False
            record(f"build {d}", False, r.stderr[-300:])
        else:
            record(f"build {d}", True)
    return ok


def run(binary, args=None, stdin="", timeout=30):
    r = subprocess.run([binary] + (args or []), input=stdin,
                       capture_output=True, text=True, timeout=timeout)
    return r.returncode, r.stdout, r.stderr


# ------------------------------------------------------------------ references
def ref_filter(s, data):
    return data.replace(s, "*" * len(s))


def ref_n_queens(n):
    res = []
    q = [-1] * n

    def conflict(col, row):
        for c in range(col):
            if q[c] == row or abs(q[c] - row) == col - c:
                return True
        return False

    def solve(col):
        if col == n:
            res.append(tuple(q))
            return
        for row in range(n):
            if not conflict(col, row):
                q[col] = row
                solve(col + 1)
    if n > 0:
        solve(0)
    return set(res)


def ref_permutations(s):
    s_sorted = "".join(sorted(s))
    return sorted("".join(p) for p in itertools.permutations(s_sorted))


def ref_powerset(n, arr):
    res = set()
    m = len(arr)
    for r in range(1, m + 1):
        for combo in itertools.combinations(range(m), r):
            if sum(arr[i] for i in combo) == n:
                res.add(" ".join(str(arr[i]) for i in combo))
    return res


def ref_rip(s):
    # minimum removals to balance
    bal = 0
    min_right = 0
    for ch in s:
        if ch == "(":
            bal += 1
        elif ch == ")":
            if bal > 0:
                bal -= 1
            else:
                min_right += 1
    min_rem = min_right + bal

    res = set()
    n = len(s)

    def valid(t):
        b = 0
        for ch in t:
            if ch == "(":
                b += 1
            elif ch == ")":
                b -= 1
                if b < 0:
                    return False
        return b == 0

    def dfs(pos, t, bal_, rem):
        if pos == n:
            if bal_ == 0 and rem == min_rem:
                res.add(t)
            return
        if s[pos] == "(":
            dfs(pos + 1, t + "(", bal_ + 1, rem)
            if rem < min_rem:
                dfs(pos + 1, t + " ", bal_, rem + 1)
        elif s[pos] == ")":
            if bal_ > 0:
                dfs(pos + 1, t + ")", bal_ - 1, rem)
            if rem < min_rem:
                dfs(pos + 1, t + " ", bal_, rem + 1)
        else:
            dfs(pos + 1, t + s[pos], bal_, rem)

    dfs(0, "", 0, 0)
    return res


def ref_tsp(points):
    if len(points) <= 1:
        return 0.0
    import math
    n = len(points)

    def dist(a, b):
        return math.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2)

    best = float("inf")
    for perm in itertools.permutations(range(1, n)):
        total = dist(points[0], points[perm[0]])
        for i in range(len(perm) - 1):
            total += dist(points[perm[i]], points[perm[i + 1]])
        total += dist(points[perm[-1]], points[0])
        if total < best:
            best = total
    return best


# ------------------------------------------------------------------ tests
def test_filter():
    print("\n>> filter")
    cases = [
        ("abc", "abcdefgaaaabcdefabc"),
        ("bonjour", "bonjour world bonjour"),
        ("a", "aaa"),
        ("ab", "ababab"),
        ("xyz", "no match here"),
        ("", "should not run"),  # empty arg -> exit 1
        ("o", "one two three"),
    ]
    for s, data in cases:
        if s == "":
            rc, out, err = run(BIN["filter"], [s], stdin=data)
            record(f"filter empty arg '{s}'", rc == 1, f"rc={rc}")
            continue
        rc, out, err = run(BIN["filter"], [s], stdin=data)
        exp = ref_filter(s, data)
        record(f"filter '{s}'", rc == 0 and out == exp,
               f"rc={rc} got={out!r} exp={exp!r}")
    # no arg
    rc, out, err = run(BIN["filter"], [], stdin="x")
    record("filter no arg", rc == 1, f"rc={rc}")
    # multiple args
    rc, out, err = run(BIN["filter"], ["a", "b"], stdin="x")
    record("filter multiple args", rc == 1, f"rc={rc}")


def test_ft_scanf():
    print("\n>> ft_scanf")
    cases = [
        ("%d", "42"),
        ("%s", "hello world"),
        ("%c", "X"),
        ("%d %s %c", "42 hello X"),
        ("%c %d %s", "Z 123 world"),
        ("%s %d", "foo 7"),
        ("%d %d", "10 20"),
        ("%s %s", "one two"),
        ("%d", "-42"),
        ("%d", "+99"),
        ("%c %c", "AB"),
        ("%s %s %s", "a b c"),
        ("%d %s", "5 apples"),
        ("%s %c", "hi Z"),
        ("%c %s", "Q test"),
        ("%s %d %c", "a 3 #"),
        ("%c %s %d", "! foo 9"),
    ]
    for fmt, data in cases:
        with open("/tmp/fts_in.txt", "w") as f:
            f.write(data)
        r = subprocess.run([BIN["ft_scanf"], fmt],
                           stdin=open("/tmp/fts_in.txt"),
                           capture_output=True, text=True, timeout=10)
        ok = r.returncode == 0 and "MATCH" in r.stdout and "MISMATCH" not in r.stdout
        record(f"ft_scanf '{fmt}' | '{data}'", ok, r.stdout.strip()[:120])


def test_gnl():
    print("\n>> get_next_line")
    cases = {
        "simple": "line1\nline2\nline3\n",
        "no_trailing_nl": "a\nb\nc",
        "empty": "",
        "single_nl": "\n",
        "long_line": "A" * 5000 + "\nB\nC\n",
        "multi_small": "x" * 10 + "\n" + "y" * 4242 + "\nz\n",
        "blank_lines": "\n\n\n",
        "mixed": "hello\n\nworld\nlast",
    }
    for name, content in cases.items():
        path = f"/tmp/gnl_{name}.txt"
        with open(path, "w") as f:
            f.write(content)
        rc, out, err = run(BIN["gnl"], [path])
        record(f"gnl {name}", rc == 0 and out == content,
               f"rc={rc} got={out[:60]!r} exp={content[:60]!r}")


def test_n_queens():
    print("\n>> n_queens")
    for n in [1, 2, 3, 4, 5, 6, 7, 8]:
        rc, out, err = run(BIN["n_queens"], [str(n)], timeout=60)
        got = set()
        for line in out.splitlines():
            parts = line.split()
            if parts:
                got.add(tuple(int(x) for x in parts))
        exp = ref_n_queens(n)
        record(f"n_queens {n} ({len(exp)} sols)",
               rc == 0 and got == exp,
               f"rc={rc} got={len(got)} exp={len(exp)}")


def test_permutations():
    print("\n>> permutations")
    for s in ["a", "ab", "abc", "abcd", "ace", "xyz", "wxyz"]:
        rc, out, err = run(BIN["permutations"], [s], timeout=30)
        got = out.splitlines()
        exp = ref_permutations(s)
        record(f"permutations '{s}'",
               rc == 0 and got == exp,
               f"rc={rc} got={len(got)} exp={len(exp)}")


def test_powerset():
    print("\n>> powerset")
    cases = [
        (3, [1, 0, 2, 4, 5, 3]),
        (12, [5, 2, 1, 8, 4, 3, 7, 11]),
        (7, [3, 8, 2]),
        (0, [1, -1]),
        (5, [1, 2, 3, 4, 5]),
        (10, [1, 2, 3, 4, 5, 6]),
        (0, [0, 1, -1]),
    ]
    for n, arr in cases:
        args = [str(n)] + [str(x) for x in arr]
        rc, out, err = run(BIN["powerset"], args)
        got = set(out.splitlines())
        exp = ref_powerset(n, arr)
        record(f"powerset n={n} arr={arr}",
               rc == 0 and got == exp,
               f"rc={rc} got={sorted(got)} exp={sorted(exp)}")


def test_rip():
    print("\n>> rip")
    cases = [
        "( ( )",
        "( ( ( ) ( ) ( ) ) ( ) )",
        "( ) ( ) ) ( )",
        "( ( ) ( ( ) (",
        "()",
        "(",
        ")",
        "",
        "((()))",
        "(()",
        "())",
    ]
    for s in cases:
        rc, out, err = run(BIN["rip"], [s])
        got = set(out.splitlines())
        exp = ref_rip(s)
        record(f"rip '{s}'", rc == 0 and got == exp,
               f"rc={rc} got={sorted(got)} exp={sorted(exp)}")


def test_tsp():
    print("\n>> tsp")
    cases = {
        "square": [(1, 1), (0, 1), (1, 0), (0, 0)],
        "triangle": [(0, 0), (3, 0), (0, 4)],
        "two": [(0, 0), (3, 4)],
        "single": [(5, 5)],
        "five": [(0, 0), (1, 0), (2, 0), (0, 1), (1, 1)],
    }
    for name, pts in cases.items():
        stdin = "".join(f"{x}, {y}\n" for x, y in pts)
        rc, out, err = run(BIN["tsp"], stdin=stdin)
        exp = ref_tsp(pts)
        got_str = out.strip()
        try:
            got = float(got_str)
        except ValueError:
            got = None
        ok = rc == 0 and got is not None and abs(got - exp) < 0.005
        record(f"tsp {name}", ok, f"rc={rc} got={got_str} exp={exp:.2f}")


def test_arg_validation():
    print("\n>> argument validation")
    # programs requiring exactly the right args
    rc, _, _ = run(BIN["n_queens"], [])
    record("n_queens no arg", rc == 1, f"rc={rc}")
    rc, _, _ = run(BIN["permutations"], [])
    record("permutations no arg", rc == 1, f"rc={rc}")
    rc, _, _ = run(BIN["powerset"], [])
    record("powerset no arg", rc == 1, f"rc={rc}")
    rc, _, _ = run(BIN["rip"], [])
    record("rip no arg", rc == 1, f"rc={rc}")
    rc, _, _ = run(BIN["filter"], [])
    record("filter no arg", rc == 1, f"rc={rc}")


def stress():
    print("\n>> stress tests")
    # filter stress
    fail = 0
    for i in range(30):
        s = "".join(chr(97 + (i * 7 + j) % 26) for j in range(3 + i % 4))
        data = "".join(chr(97 + (i + k) % 26) for k in range(50))
        rc, out, _ = run(BIN["filter"], [s], stdin=data)
        if rc != 0 or out != ref_filter(s, data):
            fail += 1
    record("filter stress (30)", fail == 0, f"{fail} failures")

    # permutations stress
    fail = 0
    for i in range(15):
        s = "".join(chr(97 + (i + j) % 26) for j in range(4))
        rc, out, _ = run(BIN["permutations"], [s])
        if rc != 0 or out.splitlines() != ref_permutations(s):
            fail += 1
    record("permutations stress (15)", fail == 0, f"{fail} failures")

    # powerset stress
    fail = 0
    for i in range(20):
        n = i % 8
        arr = [((i + k) % 7) - 2 for k in range(5)]
        args = [str(n)] + [str(x) for x in arr]
        rc, out, _ = run(BIN["powerset"], args)
        if rc != 0 or set(out.splitlines()) != ref_powerset(n, arr):
            fail += 1
    record("powerset stress (20)", fail == 0, f"{fail} failures")

    # rip stress
    fail = 0
    for i in range(20):
        s = "".join(("()"[(i + k) % 2]) for k in range(4 + i % 4))
        rc, out, _ = run(BIN["rip"], [s])
        if rc != 0 or set(out.splitlines()) != ref_rip(s):
            fail += 1
    record("rip stress (20)", fail == 0, f"{fail} failures")

    # n_queens stress (small n)
    fail = 0
    for n in [1, 2, 3, 4, 5]:
        rc, out, _ = run(BIN["n_queens"], [str(n)], timeout=30)
        got = set(tuple(int(x) for x in l.split()) for l in out.splitlines() if l.split())
        if rc != 0 or got != ref_n_queens(n):
            fail += 1
    record("n_queens stress", fail == 0, f"{fail} failures")


def main():
    print("=" * 60)
    print("EXAM RANK 03 — COMPREHENSIVE TEST SUITE")
    print("=" * 60)
    if not build_all():
        print("\nBuild failed; aborting.")
        print_summary()
        sys.exit(1)

    test_filter()
    test_ft_scanf()
    test_gnl()
    test_n_queens()
    test_permutations()
    test_powerset()
    test_rip()
    test_tsp()
    test_arg_validation()
    stress()

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
