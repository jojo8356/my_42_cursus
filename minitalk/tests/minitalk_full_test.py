#!/usr/bin/env python3
"""
Batterie de tests complète pour minitalk (42).
Couvre : Makefile, norminette, fonctionnel, vitesse (sujet : << 1s / 100 chars),
intégrité longs messages, unicode, clients successifs, parsing/erreurs, bonus.
Lancement : python3 tests/minitalk_full_test.py   (depuis la racine du projet)
"""
import os
import re
import shutil
import signal
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)

GREEN = "\033[92m"
RED = "\033[91m"
CYAN = "\033[96m"
BOLD = "\033[1m"
END = "\033[0m"

passed = 0
failed = 0
errors = []


def check(name, ok, detail=""):
    global passed, failed
    if ok:
        passed += 1
        print(f"{GREEN}[PASS]{END} {name}" + (f"  ({detail})" if detail else ""))
    else:
        failed += 1
        errors.append(name + (f" -- {detail}" if detail else ""))
        print(f"{RED}[FAIL]{END} {name}  {detail}")


def section(title):
    print(f"\n{CYAN}{BOLD}=== {title} ==={END}")


def run(cmd, timeout=30):
    return subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)


class Server:
    """Lance ./server ou ./server_bonus et capture sa sortie."""

    def __init__(self, binary="server", outfile="/tmp/mt_server.out"):
        self.outfile = outfile
        self.f = open(outfile, "wb")
        self.proc = subprocess.Popen([f"./{binary}"], stdout=self.f, stderr=subprocess.STDOUT)
        self.pid = None
        for _ in range(50):
            time.sleep(0.05)
            with open(outfile, "rb") as f:
                out = f.read().decode(errors="replace")
            m = re.search(r"Server PID:\s*(\d+)", out)
            if m:
                self.pid = int(m.group(1))
                break
        if self.pid is None:
            raise RuntimeError("le serveur n'a pas affiche son PID")

    def output(self):
        self.f.flush()
        with open(self.outfile, "rb") as f:
            return f.read().decode(errors="replace")

    def stop(self):
        if self.proc.poll() is None:
            self.proc.send_signal(signal.SIGTERM)
            try:
                self.proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.proc.kill()
        self.f.close()


# ----------------------------------------------------------------------------
section("1. MAKEFILE")
# ----------------------------------------------------------------------------
r = run(["make", "fclean"])
check("make fclean", r.returncode == 0, r.stderr.strip()[:100])

r = run(["make"])
check("make (compilation -Wall -Wextra -Werror)", r.returncode == 0,
      (r.stderr or r.stdout).strip()[:100])
check("make n'utilise pas d'autres flags", "-Wall -Wextra -Werror" in r.stdout)

r2 = run(["make"])
check("pas de relink au 2e make", r2.returncode == 0
      and "cc " not in r2.stdout and "gcc " not in r2.stdout, r2.stdout.strip()[:100])

for exe in ("client", "server"):
    check(f"exécutable ./{exe} existe", os.path.isfile(exe) and os.access(exe, os.X_OK))

r = run(["make", "bonus"])
check("make bonus", r.returncode == 0, (r.stderr or "").strip()[:100])
for exe in ("client_bonus", "server_bonus"):
    check(f"exécutable ./{exe} existe", os.path.isfile(exe) and os.access(exe, os.X_OK))

r = run(["make", "clean"])
check("make clean supprime les .o", r.returncode == 0
      and not any(f.endswith(".o") for f in os.listdir("."))
      and not os.path.exists("obj"))

r = run(["make", "re"])
check("make re", r.returncode == 0)

# ----------------------------------------------------------------------------
section("2. NORMINETTE")
# ----------------------------------------------------------------------------
if shutil.which("norminette"):
    r = subprocess.run(["norminette", "src", "include"], capture_output=True, text=True)
    out = r.stdout + r.stderr
    norm_errors = [l for l in out.splitlines() if "Error" in l]
    check("norminette OK (0 erreur)", r.returncode == 0 and not norm_errors,
          "; ".join(norm_errors[:3]))
else:
    check("norminette (ignorée si absente du PATH)", True, "non installée")

# ----------------------------------------------------------------------------
section("3. DEMARRAGE DU SERVEUR")
# ----------------------------------------------------------------------------
srv = Server("server")
check("serveur affiche son PID", srv.pid == srv.proc.pid,
      f"PID affiché={srv.pid} PID réel={srv.proc.pid}")

# ----------------------------------------------------------------------------
section("4. COMMUNICATION BASIQUE")
# ----------------------------------------------------------------------------
t0 = time.time()
r = run(["./client", str(srv.pid), "Hello, this is a first test"])
t1 = time.time()
check("message simple reçu", "Hello, this is a first test" in srv.output())
check("client quitte proprement (exit 0)", r.returncode == 0, r.stdout.strip())
check("client n'écrit pas ERROR sur un succès", "ERROR" not in r.stdout.upper())

r = run(["./client", str(srv.pid), ""])
check("chaîne vide -> juste un saut de ligne",
      srv.output().endswith("first test\n\n") and r.returncode == 0)

# ----------------------------------------------------------------------------
section("5. VITESSE (sujet : 1s pour 100 chars est 'way too much')")
# ----------------------------------------------------------------------------
msg100 = "A" * 100
t0 = time.time()
r = run(["./client", str(srv.pid), msg100])
t1 = time.time()
delta = t1 - t0
check("100 caractères en < 1s", r.returncode == 0 and delta < 1.0, f"{delta*1000:.0f} ms")
check("contenu des 100 chars intact", "A" * 100 in srv.output())

# ----------------------------------------------------------------------------
section("6. INTÉGRITÉ LONGS MESSAGES")
# ----------------------------------------------------------------------------
big = ("The quick brown fox jumps over the lazy dog. " * 200)[:5000]
r = run(["./client", str(srv.pid), big], timeout=120)
check("5 000 caractères reçus intacts", r.returncode == 0 and big in srv.output(),
      f"exit={r.returncode}")

huge = "".join(chr(97 + (i % 26)) for i in range(20000))
t0 = time.time()
r = run(["./client", str(srv.pid), huge], timeout=180)
t1 = time.time()
check("20 000 caractères reçus intacts", r.returncode == 0 and huge in srv.output(),
      f"exit={r.returncode}, {t1-t0:.1f}s")

# ----------------------------------------------------------------------------
section("7. UNICODE / CARACTÈRES SPÉCIAUX")
# ----------------------------------------------------------------------------
uni = "héllo wörld àéîõü çñ 🦊🚀 café crème"
r = run(["./client", str(srv.pid), uni])
check("unicode (accents + emojis) reçu intact", r.returncode == 0 and uni in srv.output())

special = 'ligne1\nligne2\tavec\ttabs "quotes" \'simple\' 100% \\backslash\\ {braces} [crochets]'
r = run(["./client", str(srv.pid), special])
check("caractères spéciaux (\n \t \" ' % \\ {}) intacts",
      r.returncode == 0 and special in srv.output())

pipe = "a; b && c | d > e < f $(echo pwn) `id` #comment"
r = run(["./client", str(srv.pid), pipe])
check("métacaractères shell intacts (argv)", r.returncode == 0 and pipe in srv.output())

# ----------------------------------------------------------------------------
section("8. PLUSIEURS CLIENTS SUCCESSIFS (mêmeserveur non redémarré)")
# ----------------------------------------------------------------------------
all_ok = True
for i in range(8):
    r = run(["./client", str(srv.pid), f"client-numero-{i}"])
    if r.returncode != 0:
        all_ok = False
time.sleep(0.2)
out = srv.output()
for i in range(8):
    if f"client-numero-{i}" not in out:
        all_ok = False
check("8 clients d'affilée sur le même serveur", all_ok)
check("serveur toujours vivant après tous les tests", srv.proc.poll() is None)

# ----------------------------------------------------------------------------
section("9. PARSING / ERREURS (même grille que les testers de la communauté)")
# ----------------------------------------------------------------------------
srv.stop()
bad_tests = [
    ["./client", "1234"],                       # argc != 3
    ["./client", "0"],                          # argc + pid 0
    ["./client", "154", "12134"],               # pid inexistant
    ["./client", "554", "5773", "44130"],       # trop d'arguments
    ["./client"],                               # aucun argument
    ["./client", "2147483648", "hehe"],         # pid > INT_MAX
    ["./client", "hehe", "hehe"],               # pid non numérique
    ["./client", "-2147483649", "hehe"],        # pid négatif / overflow
    ["./client", "0", "hehe"],                  # pid 0 (interdit : tue le groupe!)
    ["./client", "000", "000"],                 # pid 000 = 0
    ["./client", "-42", "hehe"],                # pid négatif
    ["./client", "12abc", "hehe"],              # pid partiellement numérique
    ["./client", "", "hehe"],                   # pid vide
    ["./client", " 42", "hehe"],                # espace devant
]
for t in bad_tests:
    r = run(t, timeout=15)
    out = (r.stdout or "") + (r.stderr or "")
    check(" ".join(t[1:]).ljust(28) + " → ERROR + exit != 0",
          "ERROR" in r.stdout.upper() and r.returncode != 0,
          (r.stdout.strip() or r.stderr.strip())[:60])

# ----------------------------------------------------------------------------
section("10. BONUS (accusé de réception + unicode)")
# ----------------------------------------------------------------------------
r = run(["make", "bonus"])
check("rebuild make bonus", r.returncode == 0, (r.stderr or "").strip()[:100])
bsrv = Server("server_bonus", "/tmp/mt_server_bonus.out")
r = run(["./client_bonus", str(bsrv.pid), "bonus: ack + unicode 🦊🚀 café"])
check("bonus: client reçoit l'ACK du serveur",
      r.returncode == 0 and "Message received by the server" in r.stdout, r.stdout.strip())
check("bonus: message unicode reçu", "bonus: ack + unicode 🦊🚀 café" in bsrv.output())

r = run(["./client_bonus", str(bsrv.pid), ""])
check("bonus: chaîne vide + ACK", r.returncode == 0
      and "Message received by the server" in r.stdout)

uni_long = "".join(chr(0x1F400 + (i % 26)) for i in range(500))  # 500 emojis
r = run(["./client_bonus", str(bsrv.pid), uni_long], timeout=180)
check("bonus: 500 emojis transmis intacts + ACK",
      r.returncode == 0 and uni_long in bsrv.output()
      and "Message received by the server" in r.stdout)

r = run(["./client_bonus", str(bsrv.pid), "x" * 3000], timeout=120)
check("bonus: 3 000 chars + ACK", r.returncode == 0
      and "x" * 3000 in bsrv.output()
      and "Message received by the server" in r.stdout)

check("bonus: serveur_bonus toujours vivant", bsrv.proc.poll() is None)
bsrv.stop()

# client_bonus avec le serveur obligatoire (sans ACK final) ne crash pas
srv2 = Server("server", "/tmp/mt_server2.out")
r = run(["./client_bonus", str(srv2.pid), "mixte"])
check("bonus: client_bonus tolère un serveur sans ACK final",
      r.returncode == 0 and "mixte" in srv2.output(), r.stdout.strip())
srv2.stop()

# ----------------------------------------------------------------------------
section("11. SÉCURITÉ PID")
# ----------------------------------------------------------------------------
r = run(["./client", "1", "hehe"], timeout=15)   # PID 1 (init) : EPERM attendu
check("PID 1 (init) rejeté sans crash", r.returncode != 0, r.stdout.strip()[:60])

# ----------------------------------------------------------------------------
print(f"\n{BOLD}{'='*60}")
print(f"RÉSULTAT : {GREEN}{passed} passed{END} / {RED}{failed} failed{END}")
if errors:
    print(f"{RED}Échecs :{END}")
    for e in errors:
        print("  -", e)
print("=" * 60 + END)
sys.exit(1 if failed else 0)
