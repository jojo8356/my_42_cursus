# Exercice 07 : Tableau périodique des éléments
# Dossier : ex07/ - Fichier : periodic_table.py
import sys

def read_elements(filepath):
    """Lit le fichier periodic_table.txt et retourne la liste des éléments."""
    elements = []
    with open(filepath, 'r') as f:
        for line in f:
            parts = line.strip().split('\t')
            if len(parts) >= 4:
                elements.append({
                    'number': parts[0],
                    'name': parts[1],
                    'symbol': parts[2],
                    'mass': parts[3]
                })
    return elements

def generate_html(elements):
    """Génère le HTML du tableau périodique."""
    html = []
    html.append('<!DOCTYPE html>')
    html.append('<html lang="fr">')
    html.append('<head>')
    html.append('    <meta charset="UTF-8">')
    html.append('    <title>Tableau périodique</title>')
    html.append('    <style>')
    html.append('        table { border-collapse: collapse; }')
    html.append('        td { border: 1px solid #333; width: 60px; height: 60px; text-align: center; vertical-align: middle; }')
    html.append('    </style>')
    html.append('</head>')
    html.append('<body>')
    html.append('    <table>')

    for el in elements:
        html.append(f'        <tr>')
        html.append(f'            <td>')
        html.append(f'                <h4>{el["name"]}</h4>')
        html.append(f'                <ul>')
        html.append(f'                    <li>Numéro atomique : {el["number"]}</li>')
        html.append(f'                    <li>Symbole : {el["symbol"]}</li>')
        html.append(f'                    <li>Masse atomique : {el["mass"]}</li>')
        html.append(f'                </ul>')
        html.append(f'            </td>')
        html.append(f'        </tr>')

    html.append('    </table>')
    html.append('</body>')
    html.append('</html>')
    return '\n'.join(html)

def main():
    if len(sys.argv) != 2:
        return
    filepath = sys.argv[1]
    elements = read_elements(filepath)
    html = generate_html(elements)
    with open('periodic_table.html', 'w') as f:
        f.write(html)

if __name__ == '__main__':
    main()
