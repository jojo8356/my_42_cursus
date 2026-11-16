# Exercice 00 : À la conquête de la Silicon Valley!
# Dossier : ex00/ - Fichiers : render.py, myCV.template, settings.py
import os
import re
import sys


def read_settings(settings_path):
    """Lit les couples clef = valeur du fichier settings.py."""
    settings = {}
    with open(settings_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or '=' not in line:
                continue
            key, value = line.split('=', 1)
            value = value.strip()
            if len(value) >= 2 and value[0] == value[-1] and value[0] in ('"', "'"):
                value = value[1:-1]
            settings[key.strip()] = value
    return settings


def render(template_path, settings_path):
    """Remplace les motifs {clef} du template par les valeurs de settings."""
    with open(template_path, 'r') as f:
        content = f.read()
    settings = read_settings(settings_path)

    def substitute(match):
        key = match.group(1)
        return settings.get(key, match.group(0))

    return re.sub(r'\{([A-Za-z_][A-Za-z0-9_]*)\}', substitute, content)


def main():
    if len(sys.argv) != 2:
        print("Usage: python3 render.py <file.template>")
        return
    template_path = sys.argv[1]
    if not template_path.endswith('.template'):
        print("Error: the file must have the .template extension.")
        return
    if not os.path.exists(template_path):
        print("Error: the file does not exist.")
        return
    settings_path = 'settings.py'
    if not os.path.exists(settings_path):
        print("Error: settings.py not found.")
        return
    result = render(template_path, settings_path)
    output_path = template_path[:-len('.template')] + '.html'
    with open(output_path, 'w') as f:
        f.write(result)


if __name__ == '__main__':
    main()
