# Exercice 07 : Tableau périodique des éléments
# Dossier : ex07/ - Fichier : periodic_table.py
import os
import re
import sys

# Frontières des periodes (numero atomique)
PERIODS = [(1, 2), (3, 10), (11, 18), (19, 36), (37, 54), (55, 86), (87, 118)]
NB_COLUMNS = 18

LINE_RE = re.compile(
    r"^(?P<name>[^=]+?)\s*=\s*position:(?P<position>\d+),\s*"
    r"number:(?P<number>\d+),\s*small:\s*(?P<small>[^,]+),\s*"
    r"molar:\s*(?P<molar>[^,]+),\s*electron:\s*(?P<electron>.+)$"
)


def period_of(number):
    for index, (start, end) in enumerate(PERIODS):
        if start <= number <= end:
            return index
    return len(PERIODS) - 1


def read_elements(filepath):
    """Lit le fichier periodic_table.txt et retourne les elements parses."""
    elements = []
    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            match = LINE_RE.match(line)
            if not match:
                continue
            elements.append({
                'name': match.group('name').strip(),
                'position': int(match.group('position')),
                'number': int(match.group('number')),
                'small': match.group('small').strip(),
                'molar': match.group('molar').strip(),
                'electron': match.group('electron').strip(),
            })
    return elements


def build_grid(elements):
    """Range les elements dans une grille periodes x colonnes."""
    grid = [[None] * NB_COLUMNS for _ in PERIODS]
    for element in elements:
        row = period_of(element['number'])
        column = element['position']
        if 0 <= row < len(grid) and 0 <= column < NB_COLUMNS:
            grid[row][column] = element
    return grid


def render_cell(element):
    if element is None:
        return '<td></td>'
    return (
        '<td style="border: 1px solid black; padding: 10px">'
        f"<h4>{element['name']}</h4>"
        '<ul>'
        f"<li>No {element['number']}</li>"
        f"<li>{element['small']}</li>"
        f"<li>{element['molar']}</li>"
        f"<li>{element['electron']} electron(s)</li>"
        '</ul>'
        '</td>'
    )


def generate_html(elements):
    """Genere la page HTML du tableau periodique."""
    grid = build_grid(elements)
    rows = []
    for row in grid:
        cells = '\n'.join(render_cell(element) for element in row)
        rows.append(f'<tr>\n{cells}\n</tr>')

    html = '\n'.join([
        '<!DOCTYPE html>',
        '<html lang="en">',
        '<head>',
        '    <meta charset="UTF-8">',
        '    <title>Periodic table</title>',
        '    <style>',
        '        table { border-collapse: collapse; }',
        '        td { vertical-align: top; }',
        '    </style>',
        '</head>',
        '<body>',
        '    <h1>Periodic table</h1>',
        '    <table>',
        *rows,
        '    </table>',
        '</body>',
        '</html>',
    ])
    return html


def main():
    filepath = sys.argv[1] if len(sys.argv) == 2 else 'periodic_table.txt'
    if not os.path.exists(filepath):
        print(f"Error: file '{filepath}' not found.")
        return
    elements = read_elements(filepath)
    html = generate_html(elements)
    with open('periodic_table.html', 'w') as f:
        f.write(html)


if __name__ == '__main__':
    main()
