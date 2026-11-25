#!/usr/bin/env python3
"""Tests for the Life project (Conway's Game of Life)."""
import os
import sys
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
LIFE = os.path.join(ROOT, "level-1", "Life", "life")

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


def run_life(stdin_data, width, height, iterations):
    r = subprocess.run([LIFE, str(width), str(height), str(iterations)],
                       input=stdin_data.encode(), capture_output=True, text=False)
    return r.stdout.decode()


def main():
    print("--- Life tests ---")
    # Example from subject: dxss 3x3 0 -> column x=1 (pen starts up, x toggles)
    out = run_life("dxss", 3, 3, 0)
    check("dxss 3x3 0", out, " O \n O \n O \n")

    # dxss 3x3 1 -> row y=1 (vertical -> horizontal)
    out = run_life("dxss", 3, 3, 1)
    check("dxss 3x3 1", out, "   \nOOO\n   \n")

    # dxss 3x3 2 -> back to column x=1
    out = run_life("dxss", 3, 3, 2)
    check("dxss 3x3 2", out, " O \n O \n O \n")

    # Blinker: horizontal line at row 1, cells (1,1),(2,1),(3,1)
    # Pen moves to (1,1): d=right s=down; x=toggle(draws (1,1));
    # d=(2,1)draw d=(3,1)draw
    cmd = "dsxdd"
    out = run_life(cmd, 5, 5, 0)
    check("blinker initial (row 1)", out,
          "     \n OOO \n     \n     \n     \n")
    out = run_life(cmd, 5, 5, 1)
    # vertical blinker centered at (2,1): rows 0,1,2 col 2
    check("blinker after 1 iter (col 2)", out,
          "  O  \n  O  \n  O  \n     \n     \n")
    out = run_life(cmd, 5, 5, 2)
    # back to horizontal row 1
    check("blinker after 2 iters", out,
          "     \n OOO \n     \n     \n     \n")

    # Dead board stays dead
    out = run_life("", 3, 3, 5)
    check("empty board", out, "   \n   \n   \n")

    # Static block (2x2) stays stable
    # Block at (1,1),(2,1),(1,2),(2,2):
    # d s -> (1,1); x=draw(1,1); d=(2,1)draw; s=(2,2)draw; a=(1,2)draw
    cmd = "dsxdsa"
    out = run_life(cmd, 5, 5, 0)
    check("block initial", out,
          "     \n OO  \n OO  \n     \n     \n")
    out = run_life(cmd, 5, 5, 3)
    check("block stable after 3 iters", out,
          "     \n OO  \n OO  \n     \n     \n")

    # Glider moves diagonally
    # Glider pattern at rows 0-2, cols 0-2:
    # .O.
    # ..O
    # OOO
    # Draw: move to (0,1), x, d=(0,2)draw, a=(0,1)draw
    # then move to (1,2), x, a=(1,1)draw
    # then move to (2,0), x, s=(2,1)draw s=(2,2)draw
    # This is complex; use a simpler approach: draw each cell.
    # Actually, let's just verify a known glider moves.
    # Glider at (0,1),(1,2),(2,0),(2,1),(2,2) on 5x5
    # After 1 iter: (0,2),(1,0),(1,1),(1,2),(2,1)
    # Commands: move to each cell, toggle, move away and back (to draw)
    # Too complex for pen. Skip glider, block+blinker cover Conway rules.

    print("\nLife: %d passed, %d failed" % (PASS, FAIL))
    return 0 if FAIL == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
