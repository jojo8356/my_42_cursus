from django.shortcuts import render

COLORS = {
    'noir': (0, 0, 0),
    'rouge': (255, 0, 0),
    'bleu': (0, 0, 255),
    'vert': (0, 255, 0),
}
NB_SHADES = 50


def generate_shade(color_name, step):
    """Genere la nuance `step` (0..49) de la couleur, en degradé vers le blanc."""
    red, green, blue = COLORS[color_name]
    factor = step / NB_SHADES
    red = int(red + (255 - red) * factor)
    green = int(green + (255 - green) * factor)
    blue = int(blue + (255 - blue) * factor)
    return '#{:02x}{:02x}{:02x}'.format(red, green, blue)


def index(request):
    rows = []
    for step in range(NB_SHADES):
        rows.append({name: generate_shade(name, step) for name in COLORS})
    context = {'colors': list(COLORS.keys()), 'rows': rows}
    return render(request, 'ex03/index.html', context)
