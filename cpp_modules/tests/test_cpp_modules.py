#!/usr/bin/env python3
"""Build and integration tests for the 37 CPP Module exercises.

Run from any directory with:
    python3 cpp_modules/tests/test_cpp_modules.py

The suite is dependency-free (Python standard library + make + a C++98
compiler). Every build happens in a temporary copy so the source tree is not
modified and generated files are removed automatically.
"""

from __future__ import print_function

import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

MODULE_ROOT = Path(__file__).resolve().parents[1]
EXPECTED_EXERCISES = 37
EXPECTED_FUNCTIONAL_CHECKS = 50
SANITIZER_CHECKS = 6
TOTAL_CHECKS = EXPECTED_EXERCISES + EXPECTED_FUNCTIONAL_CHECKS + SANITIZER_CHECKS
CXX = os.environ.get("CXX", "c++")
CXX_COMMAND = shlex.split(CXX)
BASE_FLAGS = "-Wall -Wextra -Werror -std=c++98"


class SkipCheck(Exception):
    """A check could not run because the requested sanitizer is unavailable."""


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def require_contains(text, *expected):
    missing = [item for item in expected if item not in text]
    require(not missing, "missing output fragment(s): %s\nOutput:\n%s" % (
        ", ".join(repr(item) for item in missing), text[:4000]
    ))


def require_success(result, context):
    require(result.returncode == 0,
            "%s exited %d\nstdout:\n%s\nstderr:\n%s" % (
                context, result.returncode, result.stdout[-4000:],
                result.stderr[-4000:]
            ))
    return result


def run_process(command, cwd, input_text=None, timeout=15, env=None):
    run_env = os.environ.copy()
    run_env["LC_ALL"] = "C"
    if env:
        run_env.update(env)
    return subprocess.run(
        [str(arg) for arg in command],
        cwd=str(cwd),
        input=input_text,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
        timeout=timeout,
        env=run_env,
    )


def binary_target(makefile):
    contents = makefile.read_text(encoding="utf-8")
    match = re.search(r"(?m)^NAME\s*=\s*([^\s#]+)", contents)
    if not match:
        match = re.search(r"(?m)^all\s*:\s*([^\s#]+)", contents)
    if not match:
        raise ValueError("cannot determine Makefile target in %s" % makefile)
    return match.group(1)


def exercise_id(path):
    return str(path.relative_to(MODULE_ROOT)).replace(os.sep, "/")


class CheckRunner(object):
    def __init__(self):
        self.total = 0
        self.passed = 0
        self.failed = []
        self.skipped = []

    def check(self, name, callback):
        self.total += 1
        try:
            callback()
        except SkipCheck as exc:
            self.skipped.append((name, str(exc)))
            print("  SKIP  %s: %s" % (name, exc))
        except Exception as exc:  # collect all failures before returning
            self.failed.append((name, "%s: %s" % (type(exc).__name__, exc)))
            print("  FAIL  %s" % name)
        else:
            self.passed += 1
            print("  PASS  %s" % name)


def prepare_exercise(source_dir, destination):
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(
        str(source_dir), str(destination),
        ignore=shutil.ignore_patterns("*.o", "*.a", "__pycache__"),
    )
    return destination


def build_one(source_dir, destination, cxx=CXX, flags=None, force=False):
    prepare_exercise(source_dir, destination)
    command = ["make", "-s"]
    if force:
        command.append("-B")
    command.append("CXX=" + cxx)
    if flags:
        command.append("CXXFLAGS=" + flags)
    result = run_process(command, destination, timeout=90)
    target = binary_target(destination / "Makefile")
    executable = destination / target
    if result.returncode != 0 or not executable.is_file():
        raise AssertionError(
            "make failed for %s (exit %d)\nstdout:\n%s\nstderr:\n%s" % (
                destination, result.returncode, result.stdout[-6000:],
                result.stderr[-6000:]
            )
        )
    return executable


def execute(binary, *args, **kwargs):
    input_text = kwargs.pop("input_text", None)
    cwd = kwargs.pop("cwd", binary.parent)
    timeout = kwargs.pop("timeout", 15)
    env = kwargs.pop("env", None)
    if kwargs:
        raise TypeError("unexpected execute options: %s" % sorted(kwargs))
    return run_process([binary] + list(args), cwd, input_text, timeout, env)


def make_phonebook_input(entries, commands):
    chunks = []
    for first, last, nickname, phone, secret in entries:
        chunks.extend(["ADD", first, last, nickname, phone, secret])
    chunks.extend(commands)
    return "\n".join(chunks) + "\n"


def sanitizer_probe(kind, scratch):
    flag = "-fsanitize=%s" % kind
    source = scratch / ("probe_%s.cpp" % kind)
    executable = scratch / ("probe_%s" % kind)
    source.write_text("int main() { return 0; }\n", encoding="utf-8")
    compile_result = run_process(
        CXX_COMMAND + [flag, "-fno-omit-frame-pointer", str(source), "-o", str(executable)],
        scratch, timeout=30,
    )
    if compile_result.returncode != 0:
        return False, "compiler does not support %s" % kind
    run_result = run_process([executable], scratch, timeout=15)
    if run_result.returncode != 0:
        detail = (run_result.stderr or run_result.stdout).strip().splitlines()
        return False, "runtime unavailable%s" % (
            ": " + detail[-1] if detail else ""
        )
    return True, ""


def sanitizer_check(kind, relative, scratch, *args, **kwargs):
    expected = kwargs.pop("expected", None)
    timeout = kwargs.pop("timeout", 20)
    if kwargs:
        raise TypeError("unexpected sanitizer options: %s" % sorted(kwargs))
    available, reason = sanitizer_probe(kind, scratch)
    if not available:
        raise SkipCheck(reason)

    source_dir = MODULE_ROOT / relative
    destination = scratch / "sanitized" / kind / relative
    flags = (BASE_FLAGS + " -fsanitize=%s -fno-omit-frame-pointer -g -O1" % kind)
    executable = build_one(source_dir, destination, CXX, flags, force=True)
    env = {
        ("ASAN_OPTIONS" if kind == "address" else "TSAN_OPTIONS"):
            "halt_on_error=1:detect_leaks=1" if kind == "address" else "halt_on_error=1:exitcode=66"
    }
    result = execute(executable, *args, timeout=timeout, env=env)
    require_success(result, "%s-sanitized %s" % (kind, relative))
    diagnostics = result.stderr
    require("AddressSanitizer" not in diagnostics,
            "ASan diagnostic:\n%s" % diagnostics)
    require("LeakSanitizer" not in diagnostics,
            "LSan diagnostic:\n%s" % diagnostics)
    require("ThreadSanitizer" not in diagnostics,
            "TSan diagnostic:\n%s" % diagnostics)
    if expected:
        require_contains(result.stdout, expected)


def main():
    exercise_makefiles = sorted(MODULE_ROOT.glob("cpp_*/ex*/Makefile"))
    if len(exercise_makefiles) != EXPECTED_EXERCISES:
        print("Expected %d exercise Makefiles, found %d." % (
            EXPECTED_EXERCISES, len(exercise_makefiles)
        ), file=sys.stderr)
        return 2

    runner = CheckRunner()
    with tempfile.TemporaryDirectory(prefix="cpp-modules-tests-") as temporary:
        scratch = Path(temporary)
        builds = {}

        print("\n[Build checks: %d exercises]" % EXPECTED_EXERCISES)
        for makefile in exercise_makefiles:
            source_dir = makefile.parent
            key = exercise_id(source_dir)
            destination = scratch / "build" / key

            def build_callback(src=source_dir, dest=destination, build_key=key):
                executable = build_one(src, dest, force=True)
                builds[build_key] = executable

            runner.check("build " + key, build_callback)

        def program(key, *args, **kwargs):
            executable = builds.get(key)
            require(executable is not None,
                    "exercise %s did not build" % key)
            return execute(executable, *args, **kwargs)

        functional = []

        # cpp_00
        functional.append(("megaphone: no arguments", lambda: require_contains(
            require_success(program("cpp_00/ex00"), "megaphone").stdout,
            "* LOUD AND UNBEARABLE FEEDBACK NOISE *\n")))
        functional.append(("megaphone: uppercase and join arguments", lambda: require(
            require_success(program("cpp_00/ex00", "shhhhh...", "I think the students are asleep..."),
                            "megaphone").stdout ==
            "SHHHHH... I THINK THE STUDENTS ARE ASLEEP... \n",
            "megaphone output did not uppercase/join arguments")))

        def phonebook_add_search():
            payload = make_phonebook_input(
                [("Alexandrianna", "Montgomery-Williams", "NickNameLong", "5550100", "secret")],
                ["SEARCH", "0", "EXIT"],
            )
            result = require_success(program("cpp_00/ex01", input_text=payload), "phonebook")
            require_contains(result.stdout, "Alexandri.", "NickNameL.",
                             "First name : Alexandrianna",
                             "Last name : Montgomery-Williams",
                             "Phone number : 5550100")

        functional.append(("phonebook: add, truncate, search", phonebook_add_search))

        def phonebook_invalid_contact():
            payload = "ADD\n\nLast\nNick\n555\nsecret\nSEARCH\nEXIT\n"
            output = require_success(program("cpp_00/ex01", input_text=payload), "phonebook")
            require_contains(output.stdout, "Invalid contact", "No contacts to display.")

        functional.append(("phonebook: reject empty fields", phonebook_invalid_contact))

        def phonebook_bad_index():
            payload = make_phonebook_input(
                [("Ada", "Lovelace", "Enchantress", "42", "math")],
                ["SEARCH", "8", "EXIT"],
            )
            output = require_success(program("cpp_00/ex01", input_text=payload), "phonebook")
            require_contains(output.stdout, "Invalid index.")

        functional.append(("phonebook: reject out-of-range index", phonebook_bad_index))

        def phonebook_capacity():
            contacts = [("Contact%02d" % i, "Last%02d" % i,
                         "Nick%02d" % i, str(i), "secret") for i in range(1, 10)]
            payload = make_phonebook_input(contacts, ["SEARCH", "0", "EXIT"])
            output = require_success(program("cpp_00/ex01", input_text=payload), "phonebook")
            require_contains(output.stdout, "First name : Contact09")

        functional.append(("phonebook: eight-slot capacity and overwrite", phonebook_capacity))

        def account_ledger():
            output = require_success(program("cpp_00/ex02"), "Account")
            require_contains(output.stdout, "New balance: 100", "New balance: 60",
                             "Not enough funds.", "Invalid deposit amount.",
                             "New balance: 260")

        functional.append(("Account: deposits, withdrawals and rejected amounts", account_ledger))

        # cpp_01
        def zombie_lifetime():
            output = require_success(program("cpp_01/ex00"), "zombie").stdout
            require_contains(output, "Foo: BraiiiiiiinnnzzzZ...", "Bar: BraiiiiiiinnnzzzZ...",
                             "Baz: BraiiiiiiinnnzzzZ...", "Foo destroyed", "Bar destroyed")

        functional.append(("Zombie: heap and stack lifetimes", zombie_lifetime))

        def zombie_horde():
            output = require_success(program("cpp_01/ex01"), "horde").stdout
            require(output.count("HordeZombie: BraiiiiiiinnnzzzZ...") == 5,
                    "expected 5 announcements, got %d" %
                    output.count("HordeZombie: BraiiiiiiinnnzzzZ..."))
            require(output.count("HordeZombie destroyed") == 5,
                    "expected all 5 horde objects to be destroyed")

        functional.append(("zombieHorde: count and array destruction", zombie_horde))

        def brain_references():
            output = require_success(program("cpp_01/ex02"), "brain").stdout
            addresses = re.findall(
                r"Memory address (?:of the string variable|held by stringPTR|held by stringREF): (0x[0-9a-fA-F]+)",
                output,
            )
            require(len(addresses) == 3 and len(set(addresses)) == 1,
                    "string, pointer and reference addresses differ: %r" % addresses)
            require(output.count("HI THIS IS BRAIN") == 3,
                    "expected all three string values")

        functional.append(("references and pointers share the same object", brain_references))

        def weapon_references():
            output = require_success(program("cpp_01/ex03"), "humans").stdout
            require_contains(output, "Bob attacks with their crude spiked club",
                             "Bob attacks with their some other type of club",
                             "Jim attacks with their crude spiked club",
                             "Jim attacks with their some other type of club")

        functional.append(("Weapon: HumanA reference and HumanB pointer updates", weapon_references))

        def sed_replace():
            with tempfile.TemporaryDirectory(prefix="cpp-sed-") as directory:
                work = Path(directory)
                source = work / "sample.txt"
                source.write_text("foo foo\nbar foo\n", encoding="utf-8")
                result = require_success(
                    program("cpp_01/ex04", str(source), "foo", "baz", cwd=work), "sed"
                )
                output = Path(str(source) + ".replace")
                require(output.is_file(), "replacement file was not created")
                require(output.read_text(encoding="utf-8") == "baz baz\nbar baz\n",
                        "unexpected replacement content")
                require(source.read_text(encoding="utf-8") == "foo foo\nbar foo\n",
                        "sed changed its source file")
                require(result.stdout == "" and result.stderr == "",
                        "successful sed should be quiet")

        functional.append(("sed: replace all matches without changing the source", sed_replace))

        def sed_arguments():
            no_args = program("cpp_01/ex04")
            require(no_args.returncode == 1, "sed without arguments should fail")
            require_contains(no_args.stderr, "Usage:")
            missing = program("cpp_01/ex04", "missing.txt", "x", "y")
            require(missing.returncode == 1, "sed should reject a missing input file")
            require_contains(missing.stderr, "could not read file")

        functional.append(("sed: argument and missing-file validation", sed_arguments))

        def harl_complain():
            output = require_success(program("cpp_01/ex05"), "Harl complain").stdout
            levels = ["[ DEBUG ]", "[ INFO ]", "[ WARNING ]", "[ ERROR ]"]
            positions = [output.find(level) for level in levels]
            require(all(position >= 0 for position in positions), "missing Harl level")
            require(positions == sorted(positions), "Harl levels are out of order")
            require([output.count(level) for level in levels] == [4, 3, 2, 1],
                    "complain(level) did not include all levels up to the selected one")

        functional.append(("Harl: complain dispatches through requested severity", harl_complain))

        def harl_filter():
            warning = require_success(program("cpp_01/ex06", "WARNING"), "Harl filter")
            require_contains(warning.stdout, "[ WARNING ]", "[ ERROR ]")
            require("[ DEBUG ]" not in warning.stdout and "[ INFO ]" not in warning.stdout,
                    "WARNING filter included lower levels")
            unknown = require_success(program("cpp_01/ex06", "UNKNOWN"), "Harl filter")
            require(unknown.stdout == "", "unknown filter should be silent")
            usage = program("cpp_01/ex06")
            require(usage.returncode == 1, "Harl filter without a level should fail")
            require_contains(usage.stderr, "Usage:")

        functional.append(("Harl filter: threshold, unknown level and usage", harl_filter))

        # cpp_02
        def fixed_copy():
            output = require_success(program("cpp_02/ex00"), "Fixed ex00").stdout
            zeros = re.findall(r"(?m)^0$", output)
            require(len(zeros) == 3, "copy/default raw values should all be zero")
            require_contains(output, "Copy constructor called", "Copy assignment operator called")

        functional.append(("Fixed ex00: default, copy and assignment preserve raw bits", fixed_copy))

        def fixed_conversion():
            output = require_success(program("cpp_02/ex01"), "Fixed ex01").stdout
            require_contains(output, "a is 1234.43", "b is 10", "c is 42.4219",
                             "d is 10", "a is 1234 as integer",
                             "c is 42 as integer")

        functional.append(("Fixed ex01: integer/float conversion and rounding", fixed_conversion))

        def fixed_operators():
            output = require_success(program("cpp_02/ex02"), "Fixed ex02").stdout
            require_contains(output, "0.00390625", "0.0078125", "10.1016")
            lines = output.splitlines()
            require(len(lines) >= 7 and lines[0] == "0" and lines[2] == "0.00390625",
                    "pre/post increment sequence is incorrect")

        functional.append(("Fixed ex02: arithmetic, comparison and increment", fixed_operators))

        def bsp_inside():
            output = require_success(program("cpp_02/ex03"), "BSP").stdout
            require_contains(output, "point (1,1) inside: 1")

        functional.append(("BSP: strictly interior point", bsp_inside))

        def bsp_boundaries():
            output = require_success(program("cpp_02/ex03"), "BSP").stdout
            require_contains(output, "point (0,0) vertex: 0", "point (2,2) on edge: 0",
                             "point (5,5) outside: 0", "point (1,3) inside: 0")

        functional.append(("BSP: vertices, edges and outside points are excluded", bsp_boundaries))

        # cpp_03
        def claptrap_state():
            output = require_success(program("cpp_03/ex00"), "ClapTrap").stdout
            require_contains(output, "ClapTrap CL-42 attacks target",
                             "ClapTrap CL-42 takes 20 points of damage!",
                             "ClapTrap CL-42 is not able to attack.")

        functional.append(("ClapTrap: attacks, damage and disabled state", claptrap_state))

        def scavtrap_specialization():
            output = require_success(program("cpp_03/ex01"), "ScavTrap").stdout
            require_contains(output, "ScavTrap SC-42 attacks target",
                             "causing 20 points of damage!", "Gate keeper mode")

        functional.append(("ScavTrap: specialized attack and gate mode", scavtrap_specialization))

        def fragtrap_specialization():
            output = require_success(program("cpp_03/ex02"), "FragTrap").stdout
            require_contains(output, "FragTrap FR-42 attacks target",
                             "causing 30 points of damage!", "requests high fives")

        functional.append(("FragTrap: specialized attack and high-five request", fragtrap_specialization))

        def diamondtrap_multiple_inheritance():
            output = require_success(program("cpp_03/ex03"), "DiamondTrap").stdout
            require_contains(output, "DiamondTrap DI-42 attacks target",
                             "causing 30 points of damage!", "Gate keeper mode",
                             "requests high fives")

        functional.append(("DiamondTrap: combined abilities and single shared base", diamondtrap_multiple_inheritance))

        # cpp_04
        def animal_polymorphism():
            output = require_success(program("cpp_04/ex00"), "Animal ex00").stdout
            require_contains(output, "Dog ", "Cat ", "Meow", "Woof",
                             "...animal sound...", "WrongCat", "...wrong animal sound...")

        functional.append(("Animal: virtual dispatch and non-virtual contrast", animal_polymorphism))

        def brain_deep_copy_ex01():
            output = require_success(program("cpp_04/ex01"), "Animal ex01").stdout
            require_contains(output, "original idea: I want a bone",
                             "copy idea: I want a fish")
            require(output.count("Woof") == 5 and output.count("Meow") == 5,
                    "expected five Dogs and five Cats to make sounds")

        functional.append(("Animal ex01: Brain ideas are deep-copied", brain_deep_copy_ex01))

        def brain_deep_copy_ex02():
            output = require_success(program("cpp_04/ex02"), "Animal ex02").stdout
            require_contains(output, "original idea: I want a bone",
                             "copy idea: I want a fish")
            require("Brain destructor called" in output,
                    "Brain destructors were not run")

        functional.append(("Animal ex02: deep copy and virtual destruction", brain_deep_copy_ex02))

        def materia_usage():
            output = require_success(program("cpp_04/ex03"), "Materia").stdout
            require_contains(output, "* shoots an ice bolt at Bob *",
                             "* heals Bob's wounds *",
                             "createMateria unknown returned NULL as expected")

        functional.append(("Materia: clone, equip, use and unknown type", materia_usage))

        # cpp_05
        def bureaucrat_boundaries():
            result = require_success(program("cpp_05/ex00"), "Bureaucrat")
            output = result.stdout + result.stderr
            require_contains(output, "John, bureaucrat grade 10.",
                             "Exception: Grade too high!", "Exception: Grade too low!")

        functional.append(("Bureaucrat: grade range and increment/decrement exceptions", bureaucrat_boundaries))

        def form_signing():
            output = require_success(program("cpp_05/ex01"), "Form").stdout
            require_contains(output, "John signed Form MyForm", "signed: yes",
                             "Jane couldn't sign Form HardForm", "Grade too low!")

        functional.append(("Form: authorized and rejected signatures", form_signing))

        def abstract_forms():
            with tempfile.TemporaryDirectory(prefix="cpp-forms-") as directory:
                output = require_success(program("cpp_05/ex02", cwd=directory), "AForm").stdout
                require_contains(output, "Form is not signed!", "John signed Form",
                                 "has been pardoned by Zaphod Beeblebrox.")
                shrubbery = Path(directory) / "home_shrubbery"
                require(shrubbery.is_file(), "ShrubberyCreationForm did not create its file")
                require("* Inserting a screwdriver in the ear of robot *" in output,
                        "RobotomyRequestForm did not execute")

        functional.append(("AForm: signing, execution and shrubbery side effect", abstract_forms))

        def intern_factory():
            with tempfile.TemporaryDirectory(prefix="cpp-intern-") as directory:
                result = program("cpp_05/ex03", cwd=directory)
                require_success(result, "Intern")
                require_contains(result.stdout, "Unknown form type handled",
                                 "has been pardoned by Zaphod Beeblebrox.")
                require_contains(result.stderr, "Unknown form type: unknown type")
                require((Path(directory) / "home_shrubbery").is_file(),
                        "Intern-created shrubbery form did not create its file")

        functional.append(("Intern: create named forms and reject unknown type", intern_factory))

        # cpp_06
        def scalar_regular():
            output = require_success(program("cpp_06/ex00", "a"), "ScalarConverter").stdout
            require_contains(output, "char: 'a'", "int: 97", "float: 97f", "double: 97")

        functional.append(("ScalarConverter: character conversion", scalar_regular))

        def scalar_edges():
            nan_output = require_success(program("cpp_06/ex00", "nan"), "ScalarConverter nan").stdout
            inf_output = require_success(program("cpp_06/ex00", "+inf"), "ScalarConverter inf").stdout
            float_output = require_success(program("cpp_06/ex00", "42.0f"), "ScalarConverter float").stdout
            require_contains(nan_output, "char: impossible", "int: impossible",
                             "float: impossible", "double: nan")
            require_contains(inf_output, "char: impossible", "int: impossible", "double: inf")
            require_contains(float_output, "float: 42f", "double: 42")

        functional.append(("ScalarConverter: nan, infinity and float suffix", scalar_edges))

        def serializer_roundtrip():
            output = require_success(program("cpp_06/ex01"), "Serializer").stdout
            require_contains(output, "equal: true", "id: 42", "value: 3.14", "name: test")

        functional.append(("Serializer: pointer round-trip preserves Data", serializer_roundtrip))

        def identify_dynamic_types():
            output = require_success(program("cpp_06/ex02"), "identify").stdout
            labels = output.splitlines()
            require(len(labels) == 6 and set(labels).issubset(set(["A", "B", "C"])),
                    "expected six recognized dynamic types, got %r" % labels)

        functional.append(("Identify: dynamic_cast identifies generated types", identify_dynamic_types))

        # cpp_07
        def whatever_templates():
            output = require_success(program("cpp_07/ex00"), "whatever").stdout
            require_contains(output, "a = 3, b = 2", "min( a, b ) = 2",
                             "max( a, b ) = 3", "c = chaine2, d = chaine1",
                             "min( c, d ) = chaine1", "max( c, d ) = chaine2")

        functional.append(("Templates: swap/min/max for integers and strings", whatever_templates))

        def iter_template():
            output = require_success(program("cpp_07/ex01"), "iter").stdout
            require(output.splitlines() == ["0", "1", "2", "42", "4242", "un", "deux",
                                            "trois", "quatre", "cinq", "1", "2", "3"],
                    "iter did not visit each mutable/const element in order")

        functional.append(("iter: mutable, const and string arrays", iter_template))

        # cpp_08
        def easyfind_containers():
            output = require_success(program("cpp_08/ex00"), "easyfind").stdout
            require(output.splitlines() == ["42", "17", "Exception: Value not found in container", "9"],
                    "easyfind result or not-found handling differs")

        functional.append(("easyfind: vector, list and not-found exception", easyfind_containers))

        def span_values():
            output = require_success(program("cpp_08/ex01"), "Span").stdout
            numeric = [line for line in output.splitlines() if re.fullmatch(r"\d+", line)]
            require(numeric == ["2", "14", "2", "19998"],
                    "unexpected shortest/longest spans: %r" % numeric)

        functional.append(("Span: shortest/longest values including 10,000 entries", span_values))

        def span_capacity_exception():
            output = require_success(program("cpp_08/ex01"), "Span").stdout
            require_contains(output, "Exception: Span is full")

        functional.append(("Span: reject insertion past capacity", span_capacity_exception))

        def mutant_stack_iterators():
            output = require_success(program("cpp_08/ex02"), "MutantStack").stdout
            expected = ["17", "1", "5", "3", "5", "737", "0",
                        "17", "1", "5", "3", "5", "737", "0"]
            require(output.splitlines() == expected,
                    "MutantStack stack/iterator traversal differs: %r" % output.splitlines())

        functional.append(("MutantStack: top, pop, size and iterators", mutant_stack_iterators))

        # cpp_09
        def bitcoin_calculation():
            with tempfile.TemporaryDirectory(prefix="cpp-btc-") as directory:
                work = Path(directory)
                (work / "data.csv").write_text(
                    "date,exchange_rate\n2009-01-02,2.0\n2009-01-05,3.0\n",
                    encoding="utf-8",
                )
                source = work / "input.txt"
                source.write_text(
                    "date | value\n2009-01-02 | 2\n2009-01-03 | 3\n2009-01-05 | 4\n",
                    encoding="utf-8",
                )
                output = require_success(
                    program("cpp_09/ex00", str(source), cwd=work), "BitcoinExchange"
                ).stdout
                require_contains(output, "2009-01-02 | 2 => 4.00",
                                 "2009-01-03 | 3 => 6.00",
                                 "2009-01-05 | 4 => 12.00")

        functional.append(("BitcoinExchange: database lookup and previous-date rate", bitcoin_calculation))

        def bitcoin_invalid_rows():
            with tempfile.TemporaryDirectory(prefix="cpp-btc-invalid-") as directory:
                work = Path(directory)
                (work / "data.csv").write_text(
                    "date,exchange_rate\n2011-01-01,10.0\n", encoding="utf-8"
                )
                source = work / "input.txt"
                source.write_text(
                    "date | value\n2011-01-01 | 2\n2011-02-30 | 1\n"
                    "2011-01-01 | -1\n2011-01-01 | 1001\nbad row\n",
                    encoding="utf-8",
                )
                result = require_success(
                    program("cpp_09/ex00", str(source), cwd=work), "BitcoinExchange"
                )
                require_contains(result.stdout, "2011-01-01 | 2 => 20.00")
                require_contains(result.stderr, "bad input", "not a positive number.",
                                 "too large a number.")
                missing = program("cpp_09/ex00", str(work / "absent.txt"), cwd=work)
                require_contains(missing.stderr, "could not open file")

        functional.append(("BitcoinExchange: malformed dates, values and missing input", bitcoin_invalid_rows))

        def rpn_valid():
            output = require_success(program("cpp_09/ex01", "3 4 + 2 *"), "RPN").stdout
            require(output == "14\n", "RPN expression should evaluate to 14")

        functional.append(("RPN: valid reverse-Polish expression", rpn_valid))

        def rpn_invalid():
            malformed = require_success(program("cpp_09/ex01", "1 +"), "RPN").stdout
            division = require_success(program("cpp_09/ex01", "4 0 /"), "RPN").stdout
            no_args = require_success(program("cpp_09/ex01"), "RPN").stdout
            require(malformed == "Error\n" and division == "Error\n" and no_args == "Error\n",
                    "RPN should reject underflow, division by zero and missing input")

        functional.append(("RPN: reject malformed, divide-by-zero and missing expressions", rpn_invalid))

        def pmerge_small():
            output = require_success(
                program("cpp_09/ex02", "3", "5", "9", "7", "4"), "PmergeMe"
            ).stdout
            after = re.search(r"(?m)^After: (.*)$", output)
            require(after is not None, "PmergeMe did not print the sorted vector")
            require(after.group(1).split() == ["3", "4", "5", "7", "9"],
                    "PmergeMe small odd/even vector is not sorted")

        functional.append(("PmergeMe: sort small mixed input", pmerge_small))

        def pmerge_arguments():
            non_digit = require_success(program("cpp_09/ex02", "1", "x"), "PmergeMe").stdout
            negative = require_success(program("cpp_09/ex02", "-1"), "PmergeMe").stdout
            missing = require_success(program("cpp_09/ex02"), "PmergeMe").stdout
            require_contains(non_digit, "non-digit")
            require_contains(negative, "negative number")
            require(missing == "Error\n", "PmergeMe should reject missing values")

        functional.append(("PmergeMe: reject non-digits, negatives and no arguments", pmerge_arguments))

        def pmerge_stress(values, description):
            result = require_success(
                program("cpp_09/ex02", *[str(value) for value in values], timeout=60),
                "PmergeMe stress",
            )
            after = re.search(r"(?m)^After: (.*)$", result.stdout)
            require(after is not None, "missing sorted output")
            actual = [int(value) for value in after.group(1).split()]
            require(actual == sorted(values), "%s stress input was not sorted" % description)
            require(len(actual) == 3000, "stress sort lost input elements")

        functional.append(("PmergeMe: 3,000 distinct even integers", lambda: pmerge_stress(
            list(range(6000, 0, -2)), "even")))
        functional.append(("PmergeMe: 3,000 distinct odd integers", lambda: pmerge_stress(
            list(range(5999, 0, -2)), "odd")))

        if len(functional) != EXPECTED_FUNCTIONAL_CHECKS:
            print("Expected %d functional checks, defined %d." % (
                EXPECTED_FUNCTIONAL_CHECKS, len(functional)
            ), file=sys.stderr)
            return 2

        print("\n[Functional checks: %d scenarios]" % len(functional))
        for name, callback in functional:
            runner.check(name, callback)

        sanitized = [
            ("address", "cpp_04/ex01", (), {"expected": "original idea: I want a bone"}),
            ("address", "cpp_05/ex02", (), {"expected": "has been pardoned by Zaphod Beeblebrox."}),
            ("address", "cpp_09/ex02", tuple(str(value) for value in range(257, 0, -1)),
             {"expected": "After:"}),
            ("thread", "cpp_03/ex03", (), {"expected": "requests high fives"}),
            ("thread", "cpp_06/ex02", (), {}),
            ("thread", "cpp_08/ex01", (), {"expected": "19998"}),
        ]
        print("\n[Sanitizer checks: %d builds/runs]" % len(sanitized))
        for kind, relative, args, options in sanitized:
            name = "%s-sanitized %s" % ("ASan" if kind == "address" else "TSan", relative)
            runner.check(name, lambda k=kind, r=relative, a=args, opts=options:
                         sanitizer_check(k, r, scratch, *a, **opts))

        print("\nResult: %d/%d passed, %d failed, %d skipped." % (
            runner.passed, runner.total, len(runner.failed), len(runner.skipped)
        ))
        if runner.failed:
            print("\nFailures:")
            for name, detail in runner.failed:
                print("- %s\n  %s" % (name, detail))
        if runner.skipped:
            print("\nSkipped sanitizer checks:")
            for name, detail in runner.skipped:
                print("- %s: %s" % (name, detail))
        print("Expected total: %d checks." % TOTAL_CHECKS)
        return 1 if runner.failed or runner.total != TOTAL_CHECKS else 0


if __name__ == "__main__":
    sys.exit(main())
