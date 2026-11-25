#!/usr/bin/env python3
"""Comprehensive test suite for exam_rank_05 — 'grandes entreprises' quality."""
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
        print("  [FAIL] build %s: %s" % (label, r.stderr))
        return False
    print("  [OK] build %s" % label)
    return True


def run_compiled_test(name, test_src, srcs, include_dirs):
    global TOTAL_PASS, TOTAL_FAIL
    print("\n--- %s ---" % name)
    out_bin = "/tmp/test_%s" % name
    args = ["g++", "-std=c++98", "-Wall", "-Wextra", "-Werror"]
    for inc in include_dirs:
        args += ["-I", os.path.join(ROOT, inc)]
    args += [os.path.join(HERE, test_src)]
    for src in srcs:
        args.append(os.path.join(ROOT, src))
    args += ["-o", out_bin]
    r = subprocess.run(args, capture_output=True, text=True)
    if r.returncode != 0:
        print("  [FAIL] build %s: %s" % (name, r.stderr))
        TOTAL_FAIL += 1
        return
    r = subprocess.run([out_bin], capture_output=True, text=True)
    out = r.stdout
    parts = out.strip().split("passed, ")
    if len(parts) == 2:
        p = int(parts[0].split()[-1])
        f = int(parts[1].split()[0])
        TOTAL_PASS += p
        TOTAL_FAIL += f
        print("  %d passed, %d failed" % (p, f))
    else:
        print("  [FAIL] no summary in output: %r" % out)
        TOTAL_FAIL += 1


def run_python_test(name, script):
    global TOTAL_PASS, TOTAL_FAIL
    print("\n--- %s ---" % name)
    r = subprocess.run([sys.executable, os.path.join(HERE, script)],
                       capture_output=True, text=True)
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
        TOTAL_FAIL += 1


def run_subject_main(name, binary, cwd, expected_output=None, args=None):
    """Run the subject main program and check output matches."""
    global TOTAL_PASS, TOTAL_FAIL
    print("\n--- %s (subject main) ---" % name)
    cmd = [binary]
    if args:
        cmd += args
    r = subprocess.run(cmd, capture_output=True, text=True, cwd=cwd)
    if expected_output is not None:
        if r.stdout.strip() == expected_output.strip():
            print("  [PASS] output matches subject example")
            TOTAL_PASS += 1
        else:
            print("  [FAIL] output mismatch")
            print("    expected: %r" % expected_output)
            print("    got:      %r" % r.stdout)
            TOTAL_FAIL += 1
    else:
        # just check it runs without crash
        if r.returncode == 0:
            print("  [PASS] runs without error")
            TOTAL_PASS += 1
        else:
            print("  [FAIL] crash: %s" % r.stderr)
            TOTAL_FAIL += 1


def run_student_suite(name, cwd, binary):
    global TOTAL_PASS, TOTAL_FAIL
    print("\n--- %s ---" % name)
    script = os.path.join(cwd, "tests", "run-tests.sh")
    if not os.path.exists(script):
        print("  [SKIP] no student test script at %s" % script)
        return
    binary_path = os.path.join(cwd, binary)
    r = subprocess.run(["bash", script, binary_path],
                       capture_output=True, text=True)
    # parse final result
    for line in r.stdout.splitlines():
        line = line.strip()
        if line.startswith("✅ Passed:") or line.startswith("✅ Passé:") or "Passed:" in line:
            parts = line.split("Passed:")
            if len(parts) == 2:
                passed_str = parts[1].split(",")[0].strip().replace("✅", "").strip()
                try:
                    p = int(passed_str)
                    TOTAL_PASS += p
                except ValueError:
                    pass
        if "Failed:" in line:
            parts = line.split("Failed:")
            if len(parts) == 2:
                failed_str = parts[1].split(",")[0].strip().replace("❌", "").strip()
                try:
                    f = int(failed_str)
                    TOTAL_FAIL += f
                except ValueError:
                    pass
    # Show tail of output
    for line in r.stdout.splitlines()[-5:]:
        print(line)


def main():
    print("=" * 60)
    print("exam_rank_05 test suite")
    print("=" * 60)

    # Build all projects
    builds = [
        ("level-0/vect2", "vect2"),
        ("level-0/bigint", "bigint"),
        ("level-0/polyset", "polyset"),
        ("level-1/Life", "life"),
        ("level-1/BSQ", "bsq"),
    ]
    for subdir, label in builds:
        if not build(subdir, label):
            print("Build failed, aborting")
            return 1

    # C++ unit tests
    run_compiled_test("vect2", "test_vect2.cpp",
                      ["level-0/vect2/vect2.cpp"],
                      ["level-0/vect2"])
    run_compiled_test("bigint", "test_bigint.cpp",
                      ["level-0/bigint/bigint.cpp"],
                      ["level-0/bigint"])
    run_compiled_test("polyset", "test_polyset.cpp",
                      ["level-0/polyset/searchable_array_bag.cpp",
                       "level-0/polyset/searchable_tree_bag.cpp",
                       "level-0/polyset/set.cpp",
                       "level-0/polyset/given-code/array_bag.cpp",
                       "level-0/polyset/given-code/tree_bag.cpp"],
                      ["level-0/polyset/given-code", "level-0/polyset"])

    # Subject main programs
    run_subject_main("vect2", os.path.join(ROOT, "level-0/vect2/vect2"),
                     os.path.join(ROOT, "level-0/vect2"))
    run_subject_main("bigint", os.path.join(ROOT, "level-0/bigint/bigint"),
                     os.path.join(ROOT, "level-0/bigint"))
    run_subject_main("polyset", os.path.join(ROOT, "level-0/polyset/polyset"),
                     os.path.join(ROOT, "level-0/polyset"), args=["1", "2", "3", "4"])

    # Python integration tests
    run_python_test("Life", "test_life.py")
    run_python_test("BSQ", "test_bsq.py")

    # Student reference test suites (from top students on GitHub)
    run_student_suite("Life (student ref)", os.path.join(ROOT, "level-1/Life"),
                      "life")
    run_student_suite("BSQ (student ref)", os.path.join(ROOT, "level-1/BSQ"),
                      "bsq")

    print("\n" + "=" * 60)
    print("RESULT: %d passed, %d failed" % (TOTAL_PASS, TOTAL_FAIL))
    print("=" * 60)
    return 0 if TOTAL_FAIL == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
