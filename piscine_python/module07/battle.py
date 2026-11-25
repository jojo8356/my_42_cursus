"""Module 7, Exercise 0: Battle scene using the creature factories."""

from ex0 import AquaFactory, FlameFactory


def main() -> None:
    """Create creatures from both factories and stage a battle."""
    flame = FlameFactory()
    aqua = AquaFactory()

    base_fire = flame.create_base()
    base_fire.introduce()
    base_fire.attack()

    evolved_fire = flame.create_evolved()
    evolved_fire.introduce()
    evolved_fire.attack()

    base_water = aqua.create_base()
    base_water.introduce()
    base_water.attack()

    evolved_water = aqua.create_evolved()
    evolved_water.introduce()
    evolved_water.attack()

    print("vs.")
    print("fight!")


if __name__ == "__main__":
    main()
