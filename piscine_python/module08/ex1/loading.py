"""
Exercice 1 - Loading Programs
Vérifie les dépendances, génère des données aléatoires (numpy),
crée une visualisation (matplotlib) et sauvegarde le résultat PNG.
"""

import os
import sys

import numpy as np
import pandas as pd
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


REQUIRED_PACKAGES = ["pandas", "numpy", "matplotlib"]


def check_dependencies() -> None:
    """Vérifie la disponibilité des packages requis."""
    print("LOADING STATUS:")
    print("Checking dependencies:")
    for pkg in REQUIRED_PACKAGES:
        try:
            __import__(pkg)
            print(f"[OK] {pkg}")
        except ImportError:
            print(f"[FAIL] {pkg}")


def generate_data(points: int = 50) -> pd.DataFrame:
    """Génère des données aléatoires avec numpy."""
    return pd.DataFrame(
        {
            "x": np.random.rand(points),
            "y": np.random.rand(points),
            "value": np.random.randint(0, 100, size=points),
        }
    )


def save_visualization(df: pd.DataFrame) -> str:
    """Crée et sauvegarde un graphique sous forme de PNG."""
    fig, ax = plt.subplots()
    ax.scatter(df["x"], df["y"], c=df["value"], cmap="viridis")
    ax.set_title("Matrix Analysis")
    ax.set_xlabel("x")
    ax.set_ylabel("y")

    out_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "matrix_analysis.png")
    fig.savefig(out_path, dpi=80)
    plt.close(fig)
    return out_path


def main() -> None:
    check_dependencies()

    print("Analyzing Matrix data")
    data = generate_data()
    print(f"Processing {len(data)} data points")

    print("Generating visualization")
    out_path = save_visualization(data)

    print("Analysis complete!")
    print(f"Results saved to: {out_path}")


if __name__ == "__main__":
    main()
