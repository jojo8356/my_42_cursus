"""
Tests complets - Module 10 (Programmation fonctionnelle).
Couverture : lambdas/sorted/filter/map/max/min, fonctions d'ordre supérieur,
clôtures/nonlocal, functools (reduce/partial/lru_cache/singledispatch),
décorateurs simples et paramétrés.
"""

import pytest

from conftest import run_script


# ---------------------------------------------------------------- ex0
class TestLambdaSpells:
    def test_artifact_sorter(self, module10_ex0):
        artifacts = [
            {"name": "A", "power": 50},
            {"name": "B", "power": 97},
            {"name": "C", "power": 80},
        ]
        result = module10_ex0.artifact_sorter(artifacts)
        assert [a["power"] for a in result] == [97, 80, 50]
        assert result[0]["name"] == "B"

    def test_artifact_sorter_stable(self, module10_ex0):
        result = module10_ex0.artifact_sorter([{"name": "x", "power": 5}])
        assert result == [{"name": "x", "power": 5}]

    def test_power_filter(self, module10_ex0):
        mages = [{"name": "a", "power": 90}, {"name": "b", "power": 70}, {"name": "c", "power": 80}]
        result = module10_ex0.power_filter(mages, 80)
        assert [m["name"] for m in result] == ["a", "c"]

    def test_spell_transformer(self, module10_ex0):
        result = module10_ex0.spell_transformer(["Fireball", "Heal"])
        assert result == ["* Fireball *", "* Heal *"]

    def test_mage_stats(self, module10_ex0):
        mages = [{"name": "a", "power": 90}, {"name": "b", "power": 70}, {"name": "c", "power": 80}]
        stats = module10_ex0.mage_stats(mages)
        assert stats["max_power"] == 90
        assert stats["min_power"] == 70
        assert stats["avg_power"] == 80.0

    def test_script_runs(self, module10_ex0):
        code, out, err = run_script("module10/ex0/lambda_spells.py")
        assert code == 0, err
        assert "Testing artifact sorter" in out
        assert "Stats:" in out


# ---------------------------------------------------------------- ex1
class TestHigherMagic:
    def test_spell_combiner(self, module10_ex1):
        ex1 = module10_ex1
        fun = ex1.spell_combiner(ex1.fireball, ex1.heal)
        a, b = fun("Dragon", 50)
        assert a == "Fireball hits Dragon"
        assert b == "Heals Dragon"

    def test_power_amplifier(self, module10_ex1):
        amplified = module10_ex1.power_amplifier(module10_ex1.meteor, 10)
        assert amplified("Dragon", 3) == "Dragon was hit by 30 points of impact"

    def test_conditional_caster(self, module10_ex1):
        ex1 = module10_ex1
        caster = ex1.conditional_caster(ex1.powerful, ex1.meteor)
        assert caster("Dragon", 60) == "Dragon was hit by 60 points of impact"
        assert caster("Dragon", 40) == "Spell fizzled"

    def test_spell_sequence(self, module10_ex1):
        seq = module10_ex1.spell_sequence(
            [module10_ex1.meteor, module10_ex1.earthquake, module10_ex1.tornado]
        )
        results = seq("Dragon", 200)
        assert results == ["Dragon was hit by 200 points of impact"] * 3

    def test_script_runs(self, module10_ex1):
        code, out, err = run_script("module10/ex1/higher_magic.py")
        assert code == 0, err
        assert "Combined spell result" in out
        assert "Spell fizzled" in out


# ---------------------------------------------------------------- ex2
class TestScopeMysteries:
    def test_mage_counter_isolated_state(self, module10_ex2):
        a = module10_ex2.mage_counter()
        b = module10_ex2.mage_counter()
        assert a() == 1
        assert a() == 2
        assert b() == 1  # état indépendant

    def test_spell_accumulator(self, module10_ex2):
        acc = module10_ex2.spell_accumulator(100)
        assert acc(20) == 120
        assert acc(30) == 150

    def test_enchantment_factory(self, module10_ex2):
        frozen = module10_ex2.enchantment_factory("Frozen")
        flaming = module10_ex2.enchantment_factory("Flaming")
        assert frozen("Sword") == "Frozen Sword"
        assert flaming("gun") == "Flaming gun"

    def test_memory_vault(self, module10_ex2):
        vault = module10_ex2.memory_vault()
        vault["store"]("secret", 42)
        assert vault["recall"]("secret") == 42
        assert vault["recall"]("unknown") == "Memory not found"

    def test_script_runs(self, module10_ex2):
        code, out, err = run_script("module10/ex2/scope_mysteries.py")
        assert code == 0, err
        assert "counter_a call 1: 1" in out
        assert "Memory not found" in out


# ---------------------------------------------------------------- ex3
class TestFunctoolsArtifacts:
    def test_spell_reducer_add(self, module10_ex3):
        assert module10_ex3.spell_reducer([10, 30, 30, 20, 5, 5], "add") == 100

    def test_spell_reducer_multiply(self, module10_ex3):
        assert module10_ex3.spell_reducer([60, 20, 200], "multiply") == 240000

    def test_spell_reducer_max_min(self, module10_ex3):
        assert module10_ex3.spell_reducer([30, 20, 40], "max") == 40
        assert module10_ex3.spell_reducer([30, 20, 40], "min") == 20

    def test_spell_reducer_empty_and_unknown(self, module10_ex3):
        assert module10_ex3.spell_reducer([], "add") == 0
        with pytest.raises(ValueError):
            module10_ex3.spell_reducer([1, 2], "unknown")

    def test_partial_enchanter(self, module10_ex3):
        ench = module10_ex3.partial_enchanter(module10_ex3.enchantment)
        assert ench["enchantment"]("Dragon") == "Dragon got an attack of 50 points with a fire"

    def test_memoized_fibonacci(self, module10_ex3):
        fib = module10_ex3.memoized_fibonacci
        assert fib(0) == 0
        assert fib(1) == 1
        assert fib(10) == 55
        assert fib(15) == 610

    def test_spell_dispatcher(self, module10_ex3):
        d = module10_ex3.spell_dispatcher
        assert d(42)("x") == "42 damage"
        assert d("fireball")("x") == "fireball"
        assert d(["a", "b", "c"])("x") == "3 spells"
        with pytest.raises(ValueError):
            d({})

    def test_script_runs(self, module10_ex3):
        code, out, err = run_script("module10/ex3/functools_artifacts.py")
        assert code == 0, err
        assert "Sum: 100" in out
        assert "Fib(10): 55" in out


# ---------------------------------------------------------------- ex4
class TestDecoratorMastery:
    def test_spell_timer_preserves_name(self, module10_ex4):
        assert module10_ex4.heal.__name__ == "heal"

    def test_spell_timer_returns_result(self, module10_ex4, capsys):
        result = module10_ex4.heal("Warrior", 500)
        assert result == "Heals Warrior"
        out = capsys.readouterr().out
        assert "Spell completed in" in out

    def test_power_validator_ok_and_fail(self, module10_ex4):
        assert module10_ex4.testing_power(55, "Sword") == "Sword attacked with 55 points"
        assert module10_ex4.testing_power(5, "Sword") == "Insufficient power for this spell"

    def test_retry_spell(self, module10_ex4, capsys):
        state = [0]
        result = module10_ex4.testing_retrier(state)
        assert result == "Waaaaaaagh spelled"
        out = capsys.readouterr().out
        assert "Spell failed, retrying" in out

    def test_mage_guild(self, module10_ex4):
        guild = module10_ex4.MageGuild()
        assert guild.validate_mage_name("Mago de oz") is True
        assert guild.validate_mage_name("3Mago de oz") is False
        assert guild.cast_spell(spell_name="Expelliarmus", power=20) == "Successfully cast Expelliarmus with 20 power"
        assert guild.cast_spell(spell_name="Lumos", power=9) == "Insufficient power for this spell"

    def test_script_runs(self, module10_ex4):
        code, out, err = run_script("module10/ex4/decorator_mastery.py")
        assert code == 0, err
        assert "Insufficient power for this spell" in out
        assert "Successfully cast Expelliarmus" in out
