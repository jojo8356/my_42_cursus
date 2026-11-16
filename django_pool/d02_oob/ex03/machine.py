# Exercice 03 : Glorious coffee machine!
# Dossier : ex03/ - Fichiers : machine.py, beverages.py
import random

from beverages import HotBeverage, Coffee, Tea, Chocolate, Cappuccino


class CoffeeMachine:
    class EmptyCup(HotBeverage):
        name = "empty cup"
        price = 0.90

        def description(self):
            return "An empty cup?! Gimme my money back!"

    class BrokenMachineException(Exception):
        def __init__(self):
            super().__init__("This coffee machine has to be repaired.")

    def __init__(self):
        self.counter = 0

    def repair(self):
        self.counter = 0

    def serve(self, beverage_class):
        if self.counter >= 10:
            raise CoffeeMachine.BrokenMachineException()
        self.counter += 1
        if random.randint(0, 1) == 0:
            return beverage_class()
        return CoffeeMachine.EmptyCup()


def main():
    machine = CoffeeMachine()
    for _ in range(2):
        try:
            for _ in range(11):
                print(machine.serve(Coffee))
        except CoffeeMachine.BrokenMachineException as e:
            print(e)
        machine.repair()


if __name__ == '__main__':
    main()
