# Exercice 00 : render.py - Générateur de CV
# Dossier : ex00/ - Fichiers : render.py, myCV.template, settings.py
import sys
import os
import re

def read_template(template_path):
    """Lit le contenu du fichier template."""
    with open(template_path, 'r') as f:
        return f.read()

def read_settings(settings_path):
    """Lit les valeurs du fichier settings.py."""
    settings = {}
    with open(settings_path, 'r') as f:
        for line in f:
            line = line.strip()
            if '=' in line and not line.startswith('#'):
                key, value = line.split('=', 1)
                key = key.strip()
                value = value.strip().strip('"').strip("'")
                settings[key] = value
    return settings

def render(template_path, settings_path, output_path):
    """Remplace les motifs (key) par les valeurs de settings."""
    template = read_template(template_path)
    settings = read_settings(settings_path)

    for key, value in settings.items():
        # Remplacer (key) par la valeur
        pattern = f'({key})'
        template = re.sub(pattern, value, template)

    with open(output_path, 'w') as f:
        f.write(template)

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 render.py <file.template>")
        return

    template_path = sys.argv[1]

    if not template_path.endswith('.template'):
        print("Erreur: le fichier doit avoir l'extension .template")
        return

    if not os.path.exists(template_path):
        print("Erreur: le fichier n'existe pas")
        return

    settings_path = 'settings.py'
    if not os.path.exists(settings_path):
        print("Erreur: le fichier settings.py n'existe pas")
        return

    output_path = template_path.replace('.template', '.html')
    render(template_path, settings_path, output_path)

if __name__ == '__main__':
    main()
