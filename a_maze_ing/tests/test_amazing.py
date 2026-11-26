"""Tests complets - A-Maze-ing (génération, validation, parsing, sortie)."""

import pytest

from conftest import ROOT, run_cli

from mazegen import MazeGenerator, parse_text, to_text
from mazegen.errors import ConfigError
from mazegen.solver import maze_walk, shortest_path
from mazegen.validator import (
    cycle_rank,
    dead_ends,
    open_3x3,
    validate,
    wall_mismatch,
    open_border,
)


# ---------------------------------------------------------------- parsing
class TestParser:
    def test_valid_config(self):
        cfg = parse_text("WIDTH=20\nHEIGHT=15\nENTRY=0,0\n"
                         "EXIT=19,14\nOUTPUT_FILE=m.txt\nPERFECT=True")
        assert cfg.width == 20
        assert cfg.height == 15
        assert cfg.entry == (0, 0)
        assert cfg.exit == (19, 14)
        assert cfg.perfect is True

    def test_comments_and_blanks_ignored(self):
        cfg = parse_text("# comment\n\nWIDTH=5\n#HEIGHT=x\n"
                         "HEIGHT=4\nENTRY=0,0\nEXIT=4,3\n"
                         "OUTPUT_FILE=m.txt\nPERFECT=False")
        assert cfg.width == 5 and cfg.perfect is False

    def test_seed_and_algorithm(self):
        cfg = parse_text("WIDTH=5\nHEIGHT=5\nENTRY=0,0\nEXIT=4,4\n"
                         "OUTPUT_FILE=m.txt\nPERFECT=True\nSEED=7\n"
                         "ALGORITHM=prim")
        assert cfg.seed == 7
        assert cfg.algorithm == "prim"

    @pytest.mark.parametrize("bad", [
        "", "# only comment\n", "WIDTH=5\n", "noequals\n",
        "WIDTH=5\nHEIGHT=5\nENTRY=0,0\nEXIT=4,4\nOUTPUT_FILE=m.txt\n",
        "WIDTH=abc\nHEIGHT=5\nENTRY=0,0\nEXIT=4,4\nOUTPUT_FILE=m.txt\nPERFECT=True",
        "WIDTH=0\nHEIGHT=5\nENTRY=0,0\nEXIT=4,4\nOUTPUT_FILE=m.txt\nPERFECT=True",
        "WIDTH=5\nHEIGHT=-1\nENTRY=0,0\nEXIT=4,4\nOUTPUT_FILE=m.txt\nPERFECT=True",
        "WIDTH=5\nHEIGHT=5\nENTRY=0,0\nEXIT=99,4\nOUTPUT_FILE=m.txt\nPERFECT=True",
        "WIDTH=5\nHEIGHT=5\nENTRY=0,0\nEXIT=0,0\nOUTPUT_FILE=m.txt\nPERFECT=True",
        "WIDTH=5\nHEIGHT=5\nENTRY=0\nEXIT=4,4\nOUTPUT_FILE=m.txt\nPERFECT=True",
        "WIDTH=5\nHEIGHT=5\nENTRY=0,0\nEXIT=4,4\nOUTPUT_FILE=m.txt\nPERFECT=maybe",
        "WIDTH=5\nHEIGHT=5\nENTRY=0,0\nEXIT=4,4\nOUTPUT_FILE=m.txt\nPERFECT=True\nFOO=bar",
        "WIDTH=5\nWIDTH=6\nHEIGHT=5\nENTRY=0,0\nEXIT=4,4\nOUTPUT_FILE=m.txt\nPERFECT=True",
    ])
    def test_invalid_config(self, bad):
        with pytest.raises(ConfigError):
            parse_text(bad)


# ---------------------------------------------------------------- génération
class TestGeneratorPerfect:
    def _gen(self, w=20, h=15, seed=42):
        return MazeGenerator(w, h, (0, 0), (w - 1, h - 1),
                             perfect=True, seed=seed)

    def test_perfect_is_tree(self):
        assert cycle_rank(self._gen().maze) == 0

    def test_no_structural_problems(self):
        assert validate(self._gen().maze, True) == []

    def test_walls_coherent_and_border(self):
        m = self._gen().maze
        assert wall_mismatch(m) == []
        assert open_border(m) == []

    def test_no_3x3(self):
        assert open_3x3(self._gen().maze) == []

    def test_path_valid_and_shortest(self):
        gen = self._gen()
        ok, why, _ = maze_walk(gen.maze, gen.path)
        assert ok, why
        assert len(gen.path) == len(shortest_path(gen.maze))

    def test_deterministic_seed(self):
        a = to_text(self._gen().maze, self._gen().path)
        b = to_text(self._gen().maze, self._gen().path)
        assert a == b


class TestGeneratorBoard:
    def _gen(self, w=20, h=15, seed=42):
        return MazeGenerator(w, h, (0, 0), (w - 1, h - 1),
                             perfect=False, seed=seed)

    def test_at_least_two_loops(self):
        assert cycle_rank(self._gen().maze) >= 2

    def test_no_structural_problems(self):
        assert validate(self._gen().maze, False) == []

    def test_few_dead_ends(self):
        m = self._gen().maze
        closed = {(x, y) for y in range(m.height) for x in range(m.width)
                  if m.cell(x, y).walls == 15}
        near = {(x + dx, y + dy) for x, y in closed
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))}
        real = [d for d in dead_ends(m) if d not in near]
        assert len(real) <= 2

    def test_path_valid(self):
        gen = self._gen()
        ok, why, _ = maze_walk(gen.maze, gen.path)
        assert ok, why


class TestPattern42:
    def test_pattern_present_on_large(self):
        gen = MazeGenerator(20, 15, (0, 0), (19, 14), perfect=True, seed=1)
        assert not gen.pattern_omitted
        assert len(gen.forty_two) >= 8
        m = gen.maze
        closed = {(x, y) for y in range(m.height) for x in range(m.width)
                  if m.cell(x, y).walls == 15}
        assert gen.forty_two and set(gen.forty_two) <= closed

    def test_pattern_omitted_on_small(self):
        gen = MazeGenerator(5, 5, (0, 0), (4, 4), perfect=True, seed=1)
        assert gen.pattern_omitted


# ---------------------------------------------------------------- sortie
class TestOutput:
    def test_format(self):
        gen = MazeGenerator(6, 4, (0, 0), (5, 3), perfect=True, seed=3)
        text = to_text(gen.maze, gen.path)
        assert text.endswith("\n")
        lines = text.split("\n")
        assert lines[-1] == ""
        grid = lines[:4]
        assert all(len(r) == 6 for r in grid)
        assert all(c in "0123456789ABCDEF" for r in grid for c in r)
        assert lines[4] == ""          # empty separator line
        assert lines[5] == "0,0"
        assert lines[6] == "5,3"
        assert lines[7] == gen.path


# ---------------------------------------------------------------- CLI
class TestCLI:
    def test_generates_output_file(self, tmp_path):
        cfg = tmp_path / "c.txt"
        cfg.write_text("WIDTH=8\nHEIGHT=6\nENTRY=0,0\nEXIT=7,5\n"
                       "OUTPUT_FILE=m.txt\nPERFECT=True\nSEED=1\n")
        code, out, err = run_cli([str(cfg)], input_text="4\n")
        assert code == 0, err
        out_file = ROOT / "m.txt"
        assert out_file.exists()
        text = out_file.read_text()
        assert text.endswith("\n")
        assert "0,0" in text and "7,5" in text
        out_file.unlink()

    def test_invalid_config_error_message(self, tmp_path):
        cfg = tmp_path / "c.txt"
        cfg.write_text("WIDTH=abc\n")
        code, out, err = run_cli([str(cfg)])
        assert code == 1
        assert "error" in (err + out).lower()
        assert "Traceback" not in (err + out)

    def test_small_maze_pattern_message(self, tmp_path):
        cfg = tmp_path / "c.txt"
        cfg.write_text("WIDTH=3\nHEIGHT=3\nENTRY=0,0\nEXIT=2,2\n"
                       "OUTPUT_FILE=m.txt\nPERFECT=True\nSEED=1\n")
        code, out, err = run_cli([str(cfg)], input_text="4\n")
        assert code == 0
        assert "42" in (out + err).lower()
        (ROOT / "m.txt").unlink(missing_ok=True)
