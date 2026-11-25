"""
Module 10 - Exercice 0 : Lambda Spells
Programmation fonctionnelle : lambdas, sorted(), filter(), map(),
max() et min() avec fonctions clé.
"""

from typing import Dict, List


def artifact_sorter(artifacts: list[dict]) -> list[dict]:
    """Trier les artefacts par puissance décroissante."""
    return sorted(artifacts, key=lambda a: a["power"], reverse=True)


def power_filter(mages: list[dict], min_power: int) -> list[dict]:
    """Filtrer les mages dont la puissance est >= min_power."""
    return list(filter(lambda m: m["power"] >= min_power, mages))


def spell_transformer(spells: list[str]) -> list[str]:
    """Transforme chaque sort en une ligne encadrée."""
    return list(map(lambda s: "* " + s + " *", spells))


def mage_stats(mages: list[dict]) -> dict:
    """Calculer les statistiques de puissance des mages."""
    return {
        "max_power": max(mages, key=lambda m: m["power"])["power"],
        "min_power": min(mages, key=lambda m: m["power"])["power"],
        "avg_power": round(sum(m["power"] for m in mages) / len(mages), 2),
    }


def _sample_data() -> tuple[list[dict], list[dict], list[str]]:
    """Données fictives pour la démonstration autonome."""
    artifacts = [
        {"name": "Staff of Light", "power": 92},
        {"name": "Amulet of Flame", "power": 85},
        {"name": "Ring of Shadows", "power": 97},
    ]
    mages = [
        {"name": "Elden", "power": 88},
        {"name": "Mira", "power": 74},
        {"name": "Thorn", "power": 95},
    ]
    spells = ["Fireball", "Heal", "Lightning"]
    return artifacts, mages, spells


def main() -> None:
    artifacts, mages, spells = _sample_data()

    print("Testing artifact sorter...")
    sorted_artifacts = artifact_sorter(artifacts)
    print(
        f"{sorted_artifacts[0]['name']} ({sorted_artifacts[0]['power']}) "
        f"comes before {sorted_artifacts[1]['name']} ({sorted_artifacts[1]['power']})"
    )

    print("\nTesting power filter...")
    print("As you can see all the power values are >= than the min value = 80")
    print(power_filter(mages, 80))

    print("\nTesting spell transformer...")
    print(f"Before: {spells}")
    print("After:", " ".join(spell_transformer(spells)))

    print("\nTesting mage stats...")
    print(f"Stats: {mage_stats(mages)}")


if __name__ == "__main__":
    main()
