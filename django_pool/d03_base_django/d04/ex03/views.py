from django.shortcuts import render

COLORS = ['noir', 'rouge', 'bleu', 'vert']

def generate_shade(base_color, index):
    """Génère une nuance de couleur pour un dégradé de 50 niveaux."""
    # Retourne des hex couleurs différentes
    base_rgb = {
        'noir': (0, 0, 0),
        'rouge': (255, 0, 0),
        'bleu': (0, 0, 255),
        'vert': (0, 255, 0),
    }
    r, g, b = base_rgb[base_color]
    # Interpolation vers le blanc
    factor = index / 50.0
    nr = int(r * (1 - factor))
    ng = int(g * (1 - factor))
    nb = int(b * (1 - factor))
    return f'#{nr:02x}{ng:02x}{nb:02x}'

def index(request):
    shades = []
    for color in COLORS:
        shades.append((color, [generate_shade(color, i) for i in range(50)]))
    context = {'shades': shades}
    return render(request, 'ex03/index.html', context)
