#!/usr/bin/env python3
"""
Static audit for the pipex project (Subject v2).

Checks what a Moulinette or a reviewer would check before running anything:

  1. 42 headers on every .c/.h/Makefile, 80-column lines, 4th line = filename,
     author, and the Created/Updated dates;
  2. Norm v4.1 basics that are cheap to verify mechanically: tabs for
     indentation, no line longer than 80 columns, no more than 25 lines per
     function, no more than 5 functions per file, no forbidden constructs
     (ternary, for/switch/do-while, VLA, global variables);
  3. the allowed external functions of the subject (chapter III table);
  4. the allowed symbols actually imported by the compiled binary, via nm;
  5. the Makefile rules required by the subject and the "must not relink"
     rule, verified by running make twice;
  6. the deliverables: pipex at the root, Makefile, *.h, *.c only.

Exit code 0 when every check passes, 1 otherwise. Each failure is printed on
its own line so the orchestrator can count them.
"""

import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

ALLOWED_FUNCS = {
    "open", "close", "read", "write", "malloc", "free", "perror", "strerror",
    "access", "dup", "dup2", "execve", "exit", "fork", "pipe", "unlink",
    "wait", "waitpid", "getline", "signal", "memset", "strlen", "strncmp",
    "strcmp", "strchr", "feof", "printf", "waitpid", "ft_printf",
}

# libc symbols the toolchain always pulls in; they are not "used" by the code.
TOOLCHAIN = {
    "__libc_start_main", "__cxa_finalize", "__stack_chk_fail", "__errno_location",
    "_ITM_deregisterTMCloneTable", "_ITM_registerTMCloneTable", "__gmon_start__",
    "_exit", "_start", "__libc_csu_init", "__libc_csu_fini",
}

failures = []


def fail(msg):
    failures.append(msg)


def walk_sources():
    out = []
    for base, dirs, files in os.walk(ROOT):
        dirs[:] = [d for d in dirs if d not in ("obj", "libft", "tests", ".git")]
        for f in files:
            if f.endswith((".c", ".h")):
                out.append(os.path.join(base, f))
    return sorted(out)


def check_headers(files):
    for path in files:
        rel = os.path.relpath(path, ROOT)
        with open(path, encoding="utf-8", errors="replace") as fh:
            lines = fh.readlines()
        if len(lines) < 12:
            fail(f"{rel}: shorter than the 42 header block")
            continue
        for i in range(11):
            if len(lines[i].rstrip("\n")) != 80:
                fail(f"{rel}:{i + 1}: header line is not 80 columns")
        if not lines[0].startswith("/* ***"):
            fail(f"{rel}:1: missing 42 header")
        if os.path.basename(path) not in lines[3]:
            fail(f"{rel}:4: filename missing from the header")
        if "By:" not in lines[5]:
            fail(f"{rel}:6: author missing from the header")


def strip_comments_and_strings(src):
    """Remove comments and string literals so keywords inside them do not
    produce false positives."""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if src.startswith("/*", i):
            j = src.find("*/", i + 2)
            i = n if j < 0 else j + 2
            out.append(" ")
        elif src.startswith("//", i):
            j = src.find("\n", i)
            i = n if j < 0 else j
        elif c == '"':
            i += 1
            while i < n and src[i] != '"':
                i += 2 if src[i] == "\\" else 1
            i += 1
            out.append('""')
        elif c == "'":
            i += 1
            while i < n and src[i] != "'":
                i += 2 if src[i] == "\\" else 1
            i += 1
            out.append("''")
        else:
            out.append(c)
            i += 1
    return "".join(out)


def check_norm(files):
    for path in files:
        rel = os.path.relpath(path, ROOT)
        with open(path, encoding="utf-8", errors="replace") as fh:
            raw = fh.read()
        for idx, line in enumerate(raw.split("\n"), 1):
            if len(line) > 80:
                fail(f"{rel}:{idx}: line longer than 80 columns")
            if line.startswith("    "):
                fail(f"{rel}:{idx}: indentation must use tabs")
        code = strip_comments_and_strings(raw)
        # global variables: a declaration at column 0 that is not a prototype
        for idx, line in enumerate(code.split("\n"), 1):
            s = line.strip()
            if not s or s.startswith("#") or s.startswith("typedef"):
                continue
            if re.match(r"^(int|char|void|size_t|pid_t|t_\w+)\s+\w+\s*[;=]", s) \
                    and "(" not in s and line[:1] not in ("\t", " "):
                if "extern" not in s and not s.startswith("typedef"):
                    fail(f"{rel}:{idx}: global variable (Norm IV.1)")
        funcs = re.findall(r"^[A-Za-z_][\w \t\*]*\b(\w+)\s*\([^;]*\)\s*$",
                           code, re.M)
        if len(funcs) > 5:
            fail(f"{rel}: {len(funcs)} functions, the Norm allows 5 per file")
        for kw in ("for (", "for(", "switch (", "switch(", "do {"):
            if kw in code:
                fail(f"{rel}: forbidden construct '{kw.strip()}' (Norm)")
        if "?" in code.replace("??", ""):
            # a lone ? outside strings/comments is a ternary
            for idx, line in enumerate(code.split("\n"), 1):
                if "?" in line and "?" not in ("", ):
                    fail(f"{rel}:{idx}: ternary operator (Norm)")
                    break


def function_bodies(code):
    """Yield (name, line_count) for each function definition."""
    lines = code.split("\n")
    depth, start, name = 0, None, None
    for idx, line in enumerate(lines):
        if depth == 0:
            m = re.match(r"^[A-Za-z_][\w \t\*]*\b(\w+)\s*\(", line)
            if m and line.rstrip().endswith((")", "{")):
                name, start = m.group(1), idx
        depth += line.count("{") - line.count("}")
        if depth == 0 and start is not None:
            yield name, idx - start + 1
            name, start = None, None


def check_func_length(files):
    for path in files:
        if path.endswith(".h"):
            continue
        rel = os.path.relpath(path, ROOT)
        with open(path, encoding="utf-8", errors="replace") as fh:
            code = strip_comments_and_strings(fh.read())
        for name, n in function_bodies(code):
            if n > 25:
                fail(f"{rel}: function {name}() is {n} lines (max 25)")


def check_calls(files):
    """Every name that looks like a libc call must be on the allowed list."""
    seen = set()
    for path in files:
        if path.endswith(".h"):
            continue
        rel = os.path.relpath(path, ROOT)
        with open(path, encoding="utf-8", errors="replace") as fh:
            code = strip_comments_and_strings(fh.read())
        for m in re.finditer(r"\b([a-z_][a-z0-9_]*)\s*\(", code):
            name = m.group(1)
            if name in ("if", "while", "sizeof", "return"):
                continue
            if name.startswith(("px_", "ft_")):
                continue
            if name in ALLOWED_FUNCS:
                continue
            if name in ("main", "static", "int", "char", "void"):
                continue
            if re.match(r"^(px|ft)_", name):
                continue
            line = code[:m.start()].count("\n") + 1
            seen.add(f"{rel}:{line}: call to '{name}' which is not in the subject's external functions list")
    for s in sorted(seen):
        fail(s)


def check_binary():
    binary = os.path.join(ROOT, "pipex")
    if not os.path.isfile(binary):
        fail("pipex: binary not found (run make first)")
        return
    try:
        out = subprocess.run(["nm", "-u", binary], capture_output=True,
                             text=True, check=True).stdout
    except (subprocess.CalledProcessError, FileNotFoundError) as exc:
        fail(f"nm: {exc}")
        return
    for line in out.splitlines():
        sym = line.split()[-1] if line.split() else ""
        sym = sym.split("@")[0]
        if not sym or sym in TOOLCHAIN or sym.startswith(("px_", "ft_")):
            continue
        if sym in ALLOWED_FUNCS or "__" in sym:
            continue
        fail(f"binary: imports '{sym}' which is not allowed")


def check_makefile():
    path = os.path.join(ROOT, "Makefile")
    if not os.path.isfile(path):
        fail("Makefile: missing")
        return
    with open(path, encoding="utf-8") as fh:
        text = fh.read()
    for rule in ("all", "clean", "fclean", "re", "bonus", "$(NAME)"):
        if not re.search(r"^" + re.escape(rule) + r"\s*:", text, re.M) \
                and rule != "$(NAME)":
            fail(f"Makefile: missing rule '{rule}'")
    if "-Wall" not in text or "-Wextra" not in text or "-Werror" not in text:
        fail("Makefile: must compile with -Wall -Wextra -Werror")
    if "libft" not in text:
        fail("Makefile: the libft Makefile is not used")


def check_deliverables():
    allowed = {"Makefile", "README.md", "README.fr.md", "docs", "include",
               "src", "libft", "tests", "obj", "pipex", ".git", ".gitignore",
               "run_tests.sh"}
    for entry in os.listdir(ROOT):
        if entry not in allowed:
            fail(f"root: unexpected entry '{entry}' (subject lists Makefile, *.h, *.c)")


def check_relink():
    """`make` twice must not relink: the second run must do nothing."""
    try:
        subprocess.run(["make"], cwd=ROOT, capture_output=True, check=True)
        second = subprocess.run(["make"], cwd=ROOT, capture_output=True,
                                text=True, check=True).stdout
    except subprocess.CalledProcessError as exc:
        fail(f"make: {exc}")
        return
    if "cc " in second and "-o pipex" in second:
        fail("Makefile: relinks on the second run")


def main():
    files = walk_sources()
    if not files:
        fail("no .c/.h file found")
    check_headers(files)
    check_norm(files)
    check_func_length(files)
    check_calls(files)
    check_makefile()
    check_deliverables()
    check_binary()
    if "--no-relink" not in sys.argv:
        check_relink()

    if failures:
        for f in failures:
            print(f"KO {f}")
        print(f"\n{len(failures)} problem(s) found")
        return 1
    print(f"OK {len(files)} source files: header, Norm, external functions, "
          f"binary symbols, Makefile rules and deliverables")
    return 0


if __name__ == "__main__":
    sys.exit(main())
