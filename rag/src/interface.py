"""
Interactive terminal interface (bonus).

A lightweight ``curses`` front-end over the same pipeline used by the CLI and
the HTTP API. It provides dedicated tabs (Home, Index, Search, Answer,
Settings) and a persistent logbook so nothing disappears once printed.

The business logic lives entirely in the pipeline; this module only renders
events. It is kept intentionally simple and dependency-free (stdlib curses).
"""
import os
import sys
import curses

_HERE = os.path.dirname(os.path.abspath(__file__))
if _HERE not in sys.path:
    sys.path.insert(0, _HERE)

from config import RAGConfig  # noqa: E402
from indexer import Indexer  # noqa: E402
from retriever import Retriever  # noqa: E402
from generator import make_generator  # noqa: E402
from embedder import make_embedder  # noqa: E402
from vector_store import VectorStore  # noqa: E402
from bm25 import BM25Index  # noqa: E402

TABS = ["HOME", "INDEX", "SEARCH", "ANSWER", "SETTINGS"]


class LogBook:
    """A persistent, in-memory log of events for a single tab."""

    def __init__(self):
        self.lines = []

    def add(self, text):
        self.lines.append(text)

    def extend(self, texts):
        self.lines.extend(texts)

    def clear(self):
        self.lines = []


def _load_retriever(cfg):
    work_dir = cfg.work_dir
    if not os.path.isabs(work_dir):
        work_dir = os.path.join(os.path.dirname(_HERE), work_dir)
    embedder = make_embedder(cfg.embedder)
    bm25 = BM25Index()
    if os.path.exists(os.path.join(work_dir, "bm25.json")):
        bm25 = BM25Index.load(os.path.join(work_dir, "bm25.json"))
    vector_store = None
    if os.path.exists(os.path.join(work_dir, "vector.json")):
        vector_store = VectorStore.load(os.path.join(work_dir, "vector.json"))
    return Retriever(bm25=bm25, vector_store=vector_store, embedder=embedder)


def _do_index(cfg, log):
    work_dir = cfg.work_dir
    if not os.path.isabs(work_dir):
        work_dir = os.path.join(os.path.dirname(_HERE), work_dir)
    embedder = make_embedder(cfg.embedder)
    indexer = Indexer(repo_path=cfg.repo_path, work_dir=work_dir,
                      embedder=embedder,
                      max_chunk_size=cfg.chunk_max_size,
                      overlap=cfg.chunk_overlap)
    summary = indexer.index(build_semantic=cfg.hybrid)
    indexer.bm25.save(os.path.join(work_dir, "bm25.json"))
    if cfg.hybrid:
        indexer.vector_store.save(os.path.join(work_dir, "vector.json"))
    log.clear()
    for key, value in summary.items():
        log.add(f"{key}: {value}")


def _do_search(cfg, log, query):
    retriever = _load_retriever(cfg)
    results = retriever.search(query, k=cfg.top_k,
                               mode="hybrid" if cfg.hybrid else "bm25",
                               warn=log.add)
    log.clear()
    if not results:
        log.add("No results.")
    for r in results:
        log.add(f"[{r.rank}] {r.chunk.file_path} "
                f"(score {r.score:.3f})")


def _do_answer(cfg, log, query):
    retriever = _load_retriever(cfg)
    generator = make_generator(cfg.generator)
    results = retriever.search(query, k=cfg.top_k,
                               mode="hybrid" if cfg.hybrid else "bm25",
                               warn=log.add)
    ans = generator.generate(query, results)
    log.clear()
    log.add("ANSWER:")
    log.add(ans.text)
    for i, src in enumerate(ans.sources, start=1):
        log.add(f"  source {i}: {src.chunk.file_path}")


def run(cfg, stdscr=None):
    """Run the interactive interface. ``stdscr`` is provided by curses.wrapper."""
    if stdscr is None:
        return curses.wrapper(_run, cfg)

    curses.curs_set(0)
    stdscr.keypad(True)
    tabs = {name: LogBook() for name in TABS}
    tab_index = 0
    current = "HOME"
    stdscr.clear()

    while True:
        stdscr.erase()
        height, width = stdscr.getmaxyx()

        # Tab bar.
        stdscr.addstr(0, 0, " RAG — Against the Machine "[:width - 1])
        tab_line = "   " + "   ".join(TABS)
        stdscr.addstr(1, 0, tab_line[:width - 1])
        # Highlight current tab.
        col = 3
        for name in TABS:
            attr = curses.A_REVERSE if name == current else curses.A_NORMAL
            stdscr.addstr(1, col, name, attr)
            col += len(name) + 3

        # Log content.
        log = tabs[current]
        visible = height - 4
        for i, line in enumerate(log.lines[-visible:]):
            if i + 3 < height:
                stdscr.addstr(i + 3, 0, line[:width - 1])

        if current == "INDEX":
            stdscr.addstr(height - 2, 0,
                          "[I] index    [Esc] back to Home")
        elif current in ("SEARCH", "ANSWER"):
            stdscr.addstr(height - 2, 0,
                          "Type query then Enter    [Esc] back")
        stdscr.addstr(height - 1, 0,
                      "Left/Right: tabs  Enter: confirm  Esc: Home  q: quit")
        stdscr.refresh()

        key = stdscr.getch()
        if key in (ord("q"), 27):  # q or Esc
            if key == 27 and current != "HOME":
                current = "HOME"
                continue
            break
        if key == curses.KEY_LEFT:
            tab_index = (tab_index - 1) % len(TABS)
            current = TABS[tab_index]
        elif key == curses.KEY_RIGHT:
            tab_index = (tab_index + 1) % len(TABS)
            current = TABS[tab_index]
        elif key in (curses.KEY_ENTER, 10, 13):
            if current == "INDEX":
                _do_index(cfg, tabs["INDEX"])
            elif current in ("SEARCH", "ANSWER"):
                stdscr.erase()
                stdscr.addstr(3, 0, "Query: ")
                curses.echo()
                curses.curs_set(1)
                query = stdscr.getstr(width - 20).decode("utf-8").strip()
                curses.noecho()
                curses.curs_set(0)
                if query:
                    if current == "SEARCH":
                        _do_search(cfg, tabs["SEARCH"], query)
                    else:
                        _do_answer(cfg, tabs["ANSWER"], query)
            elif current == "SETTINGS":
                for line in cfg.to_dict().items():
                    tabs["SETTINGS"].add(f"{line[0]} = {line[1]}")
        elif key == ord("i") and current == "HOME":
            _do_index(cfg, tabs["INDEX"])
            current = "INDEX"
    return 0
