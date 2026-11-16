# Exercice 00 : Antigravity
# Dossier : ex00/ - Fichier : geohashing.py
import sys

def compute_geohash(lat, lon):
    """Calcule un géohash simple à partir de coordonnées."""
    base32 = "0123456789bcdefghjkmnpqrstuvwxyz"
    lat_interval = [-90.0, 90.0]
    lon_interval = [-180.0, 180.0]
    geohash = []
    bits = [16, 8, 4, 2, 1]
    bit = 0
    ch = 0
    even = True

    while len(geohash) < 12:
        if even:
            mid = (lon_interval[0] + lon_interval[1]) / 2
            if lon > mid:
                ch |= bits[bit]
                lon_interval[0] = mid
            else:
                lon_interval[1] = mid
        else:
            mid = (lat_interval[0] + lat_interval[1]) / 2
            if lat > mid:
                ch |= bits[bit]
                lat_interval[0] = mid
            else:
                lat_interval[1] = mid
        even = not even
        if bit < 4:
            bit += 1
        else:
            geohash.append(base32[ch])
            bit = 0
            ch = 0

    return ''.join(geohash)

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 geohashing.py <lat> <lon>")
        return
    try:
        lat = float(sys.argv[-2])
        lon = float(sys.argv[-1])
    except ValueError:
        print("Erreur: les paramètres doivent être des nombres")
        return

    geohash = compute_geohash(lat, lon)
    print(geohash)

if __name__ == '__main__':
    main()
