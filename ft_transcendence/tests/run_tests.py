#!/usr/bin/env python3
"""Comprehensive test runner for ft_transcendence - 'grandes entreprises' quality."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
os.chdir(ROOT)
sys.path.insert(0, ROOT)
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "config.settings")

import django  # noqa: E402

django.setup()

from django.test.utils import get_runner  # noqa: E402
from django.conf import settings  # noqa: E402


def main():
    print("=" * 60)
    print("ft_transcendence test suite")
    print("=" * 60)
    runner = get_runner(settings)(verbosity=2, interactive=False)
    labels = [
        "tests.test_engine",
        "tests.test_ai",
        "tests.test_bracket",
        "tests.test_accounts",
        "tests.test_games",
        "tests.test_tournaments",
        "tests.test_chat",
        "tests.test_security",
        "tests.test_student_patterns",
    ]
    failures = runner.run_tests(labels)
    print("\n" + "=" * 60)
    if failures:
        print(f"RESULT: {failures} test(s) failed")
    else:
        print("RESULT: all tests passed")
    print("=" * 60)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
