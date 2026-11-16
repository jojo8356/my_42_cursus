# Exercice 03 : Glorious coffee machine!
# Dossier : ex03/ - Fichiers : machine.py, beverages.py
import random
from beverages import HotBeverage, Coffee, Tea, Chocolate, Cappuccino


class EmptyCup(HotBeverage):
    name = "empty cup"
    price = 0.90

    def description(self):
        return "An empty cup?! Gimme my money back!"


class BrokenMachineException(Exception):
    def __init__(self):
        super().__init__("This coffee machine has to be repaired.")


class CoffeeMachine:
    def __init__(self):
        self.counter = 0
        self.broken = False

    def repair(self):
        self.broken = False
        self.counter = 0

    def serve(self, beverage_class):
        if self.broken:
            raise BrokenMachineException()
        if self.counter >= 10:
            self.broken = True
            raise BrokenMachineException()

        self.counter += 1
        if random.random() < 0.5:
            return beverage_class()
        return EmptyCup()


def main():
    machine = CoffeeMachine()

    # Première série de boissons jusqu'à panne
    try:
        for _ in range(15):
            b = machine.serve(Coffee)
            print(b)
    except BrokenMachineException as e:
        print(f"Panne: {e}")

    # Réparation
    machine.repair()

    # Deuxième série
    try:
        for _ in range(15):
            b = machine.serve(Tea)
            print(b)
    except BrokenMachineException as e:
        print(f"Panne: {e}")


if __name__ == '__main__':
    main()
