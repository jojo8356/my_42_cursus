# Exercice 01 : Pip - programme utilisant path.py
# Dossier : ex01/ - Fichiers : my_script.sh, my_program.py
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                'local_lib'))

from path import Path  # noqa: E402


def main():
    folder = Path('path_lib_demo')
    folder.mkdir_p()
    target = folder / 'hello.txt'
    target.write_text('Hello from path.py!')
    print(target.read_text())


if __name__ == '__main__':
    main()
