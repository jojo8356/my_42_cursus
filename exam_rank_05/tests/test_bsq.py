#!/usr/bin/env python3
"""Tests for the BSQ project (biggest square on a map)."""
import os
import sys
import subprocess
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
BSQ = os.path.join(ROOT, "level-1", "BSQ", "bsq")

PASS = 0
FAIL = 0


def check(name, got, expected):
    global PASS, FAIL
    if got == expected:
        PASS += 1
        print("  [PASS] %s" % name)
    else:
        FAIL += 1
        print("  [FAIL] %s\n    expected: %r\n    got:      %r" % (name, expected, got))


def run_bsq(args, stdin_data=None):
    r = subprocess.run([BSQ] + args, input=stdin_data,
                       capture_output=True, text=True)
    return r.stdout, r.stderr


def write_tmp(content):
    f = tempfile.NamedTemporaryFile("w", suffix=".bsq", delete=False)
    f.write(content)
    f.close()
    return f.name


def main():
    print("--- BSQ tests ---")
    # Example from subject: 9x27 map with obstacles, biggest square = 7x7
    example = ("9 . o x\n"
               "...........................\n"
               "....o......................\n"
               "............o..............\n"
               "...........................\n"
               "....o......................\n"
               "...............o...........\n"
               "...........................\n"
               "......o..............o.....\n"
               "..o.......o................\n")
    path = write_tmp(example)
    out, err = run_bsq([path])
    expected = (".....xxxxxxx...............\n"
                "....oxxxxxxx...............\n"
                ".....xxxxxxxo..............\n"
                ".....xxxxxxx...............\n"
                "....oxxxxxxx...............\n"
                ".....xxxxxxx...o...........\n"
                ".....xxxxxxx...............\n"
                "......o..............o.....\n"
                "..o.......o................\n")
    check("subject example", out, expected)
    os.unlink(path)

    # Simple 3x3 all empty -> biggest square 3x3
    path = write_tmp("3 . o x\n...\n...\n...\n")
    out, err = run_bsq([path])
    check("all empty 3x3", out, "xxx\nxxx\nxxx\n")
    os.unlink(path)

    # 5x5 with one obstacle at (4,2), biggest square 4x4 (top-left preferred)
    path = write_tmp("5 . o x\n.....\n.....\n....o\n.....\n.....\n")
    out, err = run_bsq([path])
    # 4x4 square at top-left (rows 0-3, cols 0-3); obstacle at (4,2) preserved
    check("5x5 obstacle bottom-right", out,
          "xxxx.\nxxxx.\nxxxxo\nxxxx.\n.....\n")
    os.unlink(path)

    # No square possible (all obstacles) -> map unchanged
    path = write_tmp("3 . o x\nooo\nooo\nooo\n")
    out, err = run_bsq([path])
    check("all obstacles unchanged", out, "ooo\nooo\nooo\n")
    os.unlink(path)

    # Invalid map: lines of different length
    path = write_tmp("2 . o x\n..\n...\n")
    out, err = run_bsq([path])
    check("invalid map (diff length)", err, "map error\n")
    check("invalid map no stdout", out, "")
    os.unlink(path)

    # Invalid map: extra content after map
    path = write_tmp("2 . o x\n..\n..\n.\n")
    out, err = run_bsq([path])
    check("invalid map (extra line)", err, "map error\n")
    os.unlink(path)

    # Invalid map: identical chars
    path = write_tmp("2 . . x\n..\n..\n")
    out, err = run_bsq([path])
    check("invalid map (identical chars)", err, "map error\n")
    os.unlink(path)

    # Multiple maps: blank line between solutions
    m1 = write_tmp("2 . o x\n..\n..\n")
    m2 = write_tmp("2 . o x\noo\noo\n")
    out, err = run_bsq([m1, m2])
    check("multiple maps separator", out, "xx\nxx\n\noo\noo\n")
    os.unlink(m1)
    os.unlink(m2)

    # Read from stdin when no args
    out, err = run_bsq([], stdin_data="2 . o x\n..\n..\n")
    check("stdin read", out, "xx\nxx\n")

    # Square tie: choose top-most then left-most
    # 4x4 map, two 2x2 squares possible: top-left and bottom-right
    # top-left should be chosen
    path = write_tmp("4 . o x\n....\n....\n..o.\n..o.\n")
    out, err = run_bsq([path])
    check("tie top-left preferred", out, "xx..\nxx..\n..o.\n..o.\n")
    os.unlink(path)

    print("\nBSQ: %d passed, %d failed" % (PASS, FAIL))
    return 0 if FAIL == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
