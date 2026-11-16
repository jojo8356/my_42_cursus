# Exercice 00 : Antigravity
# Dossier : ex00/ - Fichier : geohashing.py
import sys
import antigravity


def main():
    if len(sys.argv) != 4:
        print("Usage: python3 geohashing.py <latitude> <longitude> <datedow>")
        print("Example: python3 geohashing.py 37.421542 -122.085589 "
              "2005-05-26-10458.68")
        return
    try:
        latitude = float(sys.argv[1])
        longitude = float(sys.argv[2])
    except ValueError:
        print("Error: latitude and longitude must be floats.")
        return
    datedow = sys.argv[3].encode('utf-8')
    try:
        antigravity.geohash(latitude, longitude, datedow)
    except Exception as exc:
        print(f"Error: {exc}")


if __name__ == '__main__':
    main()
