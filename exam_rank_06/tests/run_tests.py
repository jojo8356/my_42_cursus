#!/usr/bin/env python3
"""
Top-level test runner for exam_rank_06 (mini_serv + mini_db).
Builds both projects, runs the comprehensive enterprise-grade suites, and
validates against the top-student reference testers.
"""
import os
import sys
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

TOTAL_PASS = 0
TOTAL_FAIL = 0


def build(subdir, label):
    print("Building %s..." % label)
    r = subprocess.run(["make", "-C", os.path.join(ROOT, subdir), "clean", "all"],
                       capture_output=True, text=True)
    if r.returncode != 0:
        print("  [FAIL] build %s:\n%s" % (label, r.stderr))
        return False
    print("  [OK] build %s" % label)
    return True


def run_python(name, script, args=None):
    global TOTAL_PASS, TOTAL_FAIL
    print("\n--- %s ---" % name)
    cmd = [sys.executable, "-u", os.path.join(HERE, script)]
    if args:
        cmd += args
    r = subprocess.run(cmd, capture_output=True, text=True)
    print(r.stdout, end="")
    if r.returncode == 0:
        for line in r.stdout.splitlines():
            if "passed, " in line and "failed" in line:
                parts = line.split("passed, ")
                if len(parts) == 2:
                    p = int(parts[0].split()[-1])
                    f = int(parts[1].split()[0])
                    TOTAL_PASS += p
                    TOTAL_FAIL += f
                break
    else:
        print(r.stderr, end="")
        TOTAL_FAIL += 1


def run_student_mini_serv(binary):
    """Run the top-student reference tester (cacapon test_server.py) against
    our mini_serv binary. It prints PASS lines; count them."""
    global TOTAL_PASS, TOTAL_FAIL
    print("\n--- mini_serv (student reference tester) ---")
    script = os.path.join(HERE, "student_refs", "test_server_ref.py")
    r = subprocess.run([sys.executable, script, binary],
                       capture_output=True, text=True)
    passed = 0
    failed = 0
    for line in r.stdout.splitlines():
        if line.strip().startswith("PASS:"):
            passed += 1
        elif line.strip().startswith("FAIL:"):
            failed += 1
    print(r.stdout, end="")
    if r.returncode == 0 and failed == 0:
        TOTAL_PASS += passed
    else:
        TOTAL_PASS += passed
        TOTAL_FAIL += failed if failed else 1


def run_student_mini_db():
    """Run the top-student reference tester (cacapon mini_db_tester.sh)."""
    global TOTAL_PASS, TOTAL_FAIL
    print("\n--- mini_db (student reference tester) ---")
    src_dir = os.path.join(ROOT, "mini_db")
    script = os.path.join(HERE, "student_refs", "mini_db_tester.sh")
    r = subprocess.run(["bash", script, src_dir],
                       capture_output=True, text=True)
    print(r.stdout, end="")
    if r.returncode == 0:
        # The student tester reports several ✅ checks; count a pass.
        TOTAL_PASS += 1
    else:
        print(r.stderr, end="")
        TOTAL_FAIL += 1


def main():
    print("=" * 60)
    print("exam_rank_06 test suite — mini_serv + mini_db")
    print("=" * 60)

    # Build both projects
    builds = [("mini_serv", "mini_serv"), ("mini_db", "mini_db")]
    for subdir, label in builds:
        if not build(subdir, label):
            print("Build failed, aborting")
            return 1

    # Comprehensive enterprise-grade suites
    run_python("mini_serv", "test_mini_serv.py")
    run_python("mini_db", "test_mini_db.py")

    # Top-student reference testers
    run_student_mini_serv(os.path.join(ROOT, "mini_serv", "mini_serv"))
    run_student_mini_db()

    print("\n" + "=" * 60)
    print("RESULT: %d passed, %d failed" % (TOTAL_PASS, TOTAL_FAIL))
    print("=" * 60)
    return 0 if TOTAL_FAIL == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
