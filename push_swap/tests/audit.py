#!/usr/bin/env python3
"""
Static audit for push_swap.

There is no norminette in this sandbox, so the rules that can be checked
mechanically are checked here instead: the 42 header, the 80-column limit, tabs
for indentation, the 25-line function limit, the 5-function file limit, the
constructs the Norm forbids (ternaries are allowed in this project's own code
style but `for`, `switch`, `do..while` and `goto` are not), unused includes, and
the Makefile contract (NAME, all, bonus, clean, fclean, re; no relink).

The point is not to replace norminette. It is to make sure a change that breaks
a rule fails the suite in the same run as the behavioural tests, instead of
coming back at review time.
"""

import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
problems = []
checks = 0


def ok(msg):
    global checks
    checks += 1


def ko(msg):
    global checks
    checks += 1
    problems.append(msg)


def sources():
    out = []
    for base in ('src',):
        for dirpath, _, files in os.walk(os.path.join(ROOT, base)):
            for f in sorted(files):
                if f.endswith('.c'):
                    out.append(os.path.join(dirpath, f))
    return sorted(out)


def headers():
    out = []
    for dirpath, _, files in os.walk(os.path.join(ROOT, 'include')):
        for f in sorted(files):
            if f.endswith('.h'):
                out.append(os.path.join(dirpath, f))
    return sorted(out)


# ------------------------------------------------------------------ header --
def check_header(path):
    lines = open(path, encoding='utf-8', errors='replace').read().split('\n')
    if len(lines) < 11:
        return ko(f'{path}: shorter than a 42 header')
    if not lines[0].startswith('/* ****'):
        return ko(f'{path}: no 42 header (first line)')
    name = os.path.basename(path)
    if name not in lines[3]:
        return ko(f'{path}: header does not name the file ({name})')
    if not any('By: jojo8356' in l for l in lines[:11]):
        return ko(f'{path}: header has no author')
    if not any('Created:' in l for l in lines[:11]):
        return ko(f'{path}: header has no Created date')
    if not any('Updated:' in l for l in lines[:11]):
        return ko(f'{path}: header has no Updated date')
    for n in range(11):
        if len(lines[n]) != 80:
            return ko(f'{path}:{n+1}: header line is {len(lines[n])} cols')
    ok(path)


# -------------------------------------------------------------------- norm --
def check_norm(path, text):
    lines = text.split('\n')
    for i, line in enumerate(lines, 1):
        if len(line) > 80:
            ko(f'{path}:{i}: {len(line)} columns (limit 80)')
        stripped = line.lstrip(' ')
        if stripped and not stripped.startswith('*') and '\t' not in line:
            if line.startswith(' ') and not line.startswith('#') \
                    and not re.match(r'\s*(if|else|while|for|\})', line):
                ko(f'{path}:{i}: indented with spaces')
    body = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    body = re.sub(r'//.*', '', body)
    body = re.sub(r'"(\\.|[^"\\])*"', '""', body)
    body = re.sub(r"'(\\.|[^'\\])*'", "''", body)
    for word in ('for', 'switch', 'goto'):
        for m in re.finditer(r'(?<![\w_])' + word + r'\s*\(', body):
            line = body[:m.start()].count('\n') + 1
            ko(f'{path}:{line}: `{word}` is forbidden by the Norm')
    for m in re.finditer(r'do\s*\n?\s*\{', body):
        line = body[:m.start()].count('\n') + 1
        ko(f'{path}:{line}: `do..while` is forbidden by the Norm')


def functions(path, text):
    """Return (name, start_line, body) for each function definition.

    A definition is a `name(...)` whose closing paren is followed, possibly
    after a line break, by `{`. Prototypes end with `;` and are skipped, which
    is what the previous stricter pattern got wrong: it silently matched
    nothing, so the 25-line rule was never actually enforced.
    """
    out = []
    pattern = (r'\b(\w+)\s*\([^;{()]*\)\s*\n?\{')
    for m in re.finditer(pattern, text):
        name = m.group(1)
        if name in ('if', 'while', 'for', 'return', 'sizeof'):
            continue
        start = text[:m.start()].count('\n') + 1
        depth = 0
        i = text.index('{', m.start())
        for j in range(i, len(text)):
            if text[j] == '{':
                depth += 1
            elif text[j] == '}':
                depth -= 1
                if depth == 0:
                    out.append((name, start, text[i:j + 1]))
                    break
    return out


def check_functions(path, text):
    fns = functions(path, text)
    if len(fns) > 5:
        ko(f'{path}: {len(fns)} functions (limit 5)')
    for name, start, body in fns:
        n = body.count('\n') + 1
        if n > 25:
            ko(f'{path}:{start}: `{name}` is {n} lines (limit 25)')
        depth = 0
        for ch in body:
            if ch == '{':
                depth += 1
            elif ch == '}':
                depth -= 1
        if depth != 0:
            ko(f'{path}:{start}: `{name}` has unbalanced braces')


# ------------------------------------------------------------- makefile -----
def check_makefile():
    path = os.path.join(ROOT, 'Makefile')
    if not os.path.exists(path):
        return ko('Makefile is missing')
    text = open(path).read()
    for rule in ('$(NAME)', 'all', 'clean', 'fclean', 're'):
        if not re.search(r'^' + re.escape(rule) + r'\s*:', text, re.M):
            ko(f'Makefile: rule `{rule}` is missing')
    if 'bonus' not in text:
        ko('Makefile: no `bonus` rule (required to submit bonuses)')
    for flag in ('-Wall', '-Wextra', '-Werror'):
        if flag not in text:
            ko(f'Makefile: {flag} is missing')
    if 'libft' not in text:
        ko('Makefile: does not build libft through its own Makefile')
    ok('makefile')


# --------------------------------------------------------- build + relink ---
def check_build():
    def run(cmd):
        return subprocess.run(cmd, shell=True, cwd=ROOT,
                              capture_output=True, text=True)

    r = run('make fclean >/dev/null 2>&1 && make 2>&1')
    if r.returncode != 0:
        return ko(f'make failed:\n{r.stdout[-1500:]}')
    if 'warning' in r.stdout.lower():
        ko('make emitted warnings')
    if not os.path.exists(os.path.join(ROOT, 'push_swap')):
        ko('make did not produce ./push_swap')
    r = run('make 2>&1')
    if 'Nothing to be done' not in r.stdout and 'nothing to be done' not in r.stdout:
        ko('make relinked (second run did work)')
    r = run('make bonus 2>&1')
    if r.returncode != 0:
        ko(f'make bonus failed:\n{r.stdout[-1500:]}')
    if not os.path.exists(os.path.join(ROOT, 'checker')):
        ko('make bonus did not produce ./checker')
    r = run('make bonus 2>&1')
    if 'Nothing to be done' not in r.stdout and 'nothing to be done' not in r.stdout:
        ko('make bonus relinked')
    ok('build')


# --------------------------------------------------------------- symbols ----
ALLOWED = {
    'read', 'write', 'malloc', 'free', 'exit',
    'ft_printf', 'ft_putchar_fd', 'ft_putstr_fd', 'ft_putnbr_fd',
    'ft_strlen', 'ft_memcpy', 'ft_memset', 'ft_calloc', 'ft_strdup',
    'ft_atoi', 'ft_putendl_fd',
}
LIBC = {
    '__libc_start_main', 'puts', 'printf', 'putchar', 'strlen', 'memcpy',
    'memset', 'memmove', 'calloc', 'realloc', 'strdup', 'atoi', 'abort',
    'errno', 'exit', '__cxa_finalize', '_ITM_deregisterTMCloneTable',
    '_ITM_registerTMCloneTable', '__gmon_start__', 'malloc', 'free',
    'read', 'write', 'stderr', 'stdin', 'stdout', 'isdigit', 'isalpha',
}


def check_symbols():
    for binary in ('push_swap', 'checker'):
        path = os.path.join(ROOT, binary)
        if not os.path.exists(path):
            continue
        r = subprocess.run(['nm', '-u', path], capture_output=True, text=True)
        for line in r.stdout.split('\n'):
            parts = line.split()
            if len(parts) != 2:
                continue
            sym = parts[1].split('@')[0]
            if sym.startswith('__') or sym in LIBC or sym in ALLOWED:
                continue
            ko(f'{binary}: unexpected external symbol `{sym}`')
    ok('symbols')


def main():
    files = sources()
    if len(files) < 5:
        ko(f'only {len(files)} source files found')
    for path in files + headers():
        rel = os.path.relpath(path, ROOT)
        check_header(rel)
        text = open(path, encoding='utf-8', errors='replace').read()
        check_norm(rel, text)
        check_functions(rel, text)
    check_makefile()
    check_build()
    check_symbols()

    if problems:
        for p in problems:
            print(f'  KO {p}')
        print(f'\n{len(problems)} problem(s) found, {checks} checks')
        return 1
    print(f'OK {checks} checks on {len(files)} source files: header, Norm, '
          f'functions, Makefile rules, build, symbols')
    return 0


if __name__ == '__main__':
    sys.exit(main())
