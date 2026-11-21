#!/usr/bin/env bash
# ************************************************************************** #
#  Exam Rank 04 — microshell : tests d'intégration.                      #
# ************************************************************************** #
set -u
cd "$(dirname "$0")/.."

MS=./microshell
PASS=0
FAIL=0

# run_case "nom" "stdout_attendu" "substr_stderr" "exit_attendu" args...
run_case() {
    local name="$1" exp_out="$2" exp_err="$3" exp_exit="$4"
    shift 4
    local out err code
    out=$("$MS" "$@" 2>/tmp/ms_err); code=$?
    err=$(cat /tmp/ms_err)
    if [ "$out" = "$exp_out" ] && [[ "$err" == *"$exp_err"* ]] && [ "$code" -eq "$exp_exit" ]; then
        PASS=$((PASS+1)); echo "[  OK  ] $name"
    else
        FAIL=$((FAIL+1))
        echo "[ FAIL ] $name"
        echo "    attendu out=[$exp_out] err~[$exp_err] exit=$exp_exit"
        echo "    obtenu  out=[$out] err=[$err] exit=$code"
    fi
}

echo "=== tests d'intégration microshell ==="

# 1. commande simple
run_case "echo simple" "hello" "" 0 /bin/echo hello
# 2. args multiples
run_case "echo multi args" "a b c" "" 0 /bin/echo a b c
# 3. séquence ;
run_case "séquence ;" "a
b" "" 0 /bin/echo a ";" /bin/echo b
# 4. pipe
run_case "pipe |" "hello" "" 0 /bin/echo hello "|" /usr/bin/grep hello
# 5. chaîne de pipes
run_case "chaîne pipes" "hello" "" 0 /bin/echo hello "|" /bin/cat "|" /usr/bin/grep hello
# 6. pipe avec grep sans match
run_case "pipe sans match" "" "" 1 /bin/echo hello "|" /usr/bin/grep nonexistent
# 7. cd succès + pwd
run_case "cd + pwd" "/tmp" "" 0 cd /tmp ";" /bin/pwd
# 8. cd mauvais args
run_case "cd bad args" "" "error: cd: bad arguments" 1 cd
# 9. cd échec
run_case "cd fail" "" "error: cd: cannot change directory to /no/such/dir_xyz" 1 cd /no/such/dir_xyz
# 10. execve échec
run_case "execve fail" "" "error: cannot execute /no/such/bin_xyz" 127 /no/such/bin_xyz
# 11. && succès -> exécute
run_case "&& true -> run" "ok" "" 0 /bin/true "&&" /bin/echo ok
# 12. && échec -> skip
run_case "&& false -> skip" "" "" 1 /bin/false "&&" /bin/echo nope
# 13. || échec -> exécute
run_case "|| false -> run" "recovered" "" 0 /bin/false "||" /bin/echo recovered
# 14. || succès -> skip
run_case "|| true -> skip" "" "" 0 /bin/true "||" /bin/echo nope
# 15. mixte false && a || b -> b
run_case "mixte && ||" "b" "" 0 /bin/false "&&" /bin/echo a "||" /bin/echo b
# 16. vrai && || vrai && vrai
run_case "chaine &&" "c" "" 0 /bin/true "&&" /bin/true "&&" /bin/echo c
# 17. ls | grep (exemple énoncé, dans un dir contrôlé)
mkdir -p /tmp/ms_ls_test && touch /tmp/ms_ls_test/microshell
run_case "ls | grep" "microshell" "" 0 /bin/ls /tmp/ms_ls_test "|" /usr/bin/grep microshell
rm -rf /tmp/ms_ls_test
# 18. code de sortie propagé
run_case "exit code propagé" "" "" 3 /bin/sh -c "exit 3"
# 19. echo avec argument '|' comme texte (quoté, pas séparateur)
#     (un argument égal à '|' est un séparateur per l'énoncé ; on test '|x')
run_case "arg non-sep" "|x" "" 0 /bin/echo "|x"
# 20. pipeline de 3 avec cat
run_case "pipe triple" "FDTEST" "" 0 /bin/echo FDTEST "|" /bin/cat "|" /bin/cat "|" /bin/cat

# 21. centaines de pipes avec ulimit 30 (pas d'épuisement de descripteurs)
(
    ulimit -n 30
    args=(/bin/echo FDTEST)
    for i in $(seq 1 250); do args+=("|" /bin/cat); done
    out=$("$MS" "${args[@]}" 2>/tmp/ms_err); code=$?
    if [ "$out" = "FDTEST" ] && [ "$code" -eq 0 ]; then
        echo "[  OK  ] 250 pipes (ulimit 30)"
        exit 0
    else
        echo "[ FAIL ] 250 pipes (ulimit 30): out=[$out] exit=$code err=[$(cat /tmp/ms_err)]"
        exit 1
    fi
) && PASS=$((PASS+1)) || FAIL=$((FAIL+1))

# 22. cd suivi de commande relative
mkdir -p /tmp/ms_test_dir
run_case "cd relatif + pwd" "/tmp/ms_test_dir" "" 0 cd /tmp/ms_test_dir ";" /bin/pwd
rmdir /tmp/ms_test_dir

echo ""
echo "=== $PASS tests d'intégration, $FAIL échecs ==="
if [ "$FAIL" -eq 0 ]; then
    echo "ALL INTEGRATION TESTS PASSED"
    exit 0
else
    echo "SOME INTEGRATION TESTS FAILED"
    exit 1
fi
