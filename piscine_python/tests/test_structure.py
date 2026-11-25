"""
Test de régression de structure - vérifie l'arborescence complète
des 11 modules de la Piscine Python (modules 00 à 10).
"""

from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parent.parent

# Fichiers attendus par module (sans __pycache__ ni artifacts générés)
EXPECTED = {
    "module00": [
        "ex0/ft_hello_garden.py",
        "ex1/ft_garden_name.py",
        "ex2/ft_plot_area.py",
        "ex3/ft_harvest_total.py",
        "ex4/ft_plant_age.py",
        "ex5/ft_water_reminder.py",
        "ex6/ft_count_harvest_iterative.py",
        "ex6/ft_count_harvest_recursive.py",
        "ex7/ft_seed_inventory.py",
    ],
    "module01": [
        "ex0/ft_garden_intro.py",
        "ex1/ft_garden_data.py",
        "ex2/ft_plant_growth.py",
        "ex3/ft_plant_factory.py",
        "ex4/ft_garden_security.py",
        "ex5/ft_plant_types.py",
        "ex6/ft_garden_analytics.py",
    ],
    "module02": [
        "ex0/ft_first_exception.py",
        "ex1/ft_raise_exception.py",
        "ex2/ft_different_errors.py",
        "ex3/ft_custom_errors.py",
        "ex4/ft_finally_block.py",
    ],
    "module03": [
        "ex0/ft_command_quest.py",
        "ex1/ft_score_analytics.py",
        "ex2/ft_coordinate_system.py",
        "ex3/ft_achievement_tracker.py",
        "ex4/ft_inventory_system.py",
        "ex5/ft_data_stream.py",
        "ex6/ft_data_alchemist.py",
    ],
    "module04": [
        "ex0/ft_ancient_text.py",
        "ex1/ft_archive_creation.py",
        "ex2/ft_stream_management.py",
        "ex3/ft_vault_security.py",
    ],
    "module05": [
        "ex0/data_processor.py",
        "ex1/data_stream.py",
        "ex2/data_pipeline.py",
    ],
    "module06": [
        "elements.py",
        "alchemy/__init__.py",
        "alchemy/elements.py",
        "alchemy/potions.py",
        "alchemy/transmutation/recipes.py",
        "alchemy/grimoire/__init__.py",
        "alchemy/grimoire/light_validator.py",
        "alchemy/grimoire/dark_validator.py",
        "alchemy/grimoire/light_spellbook.py",
        "alchemy/grimoire/dark_spellbook.py",
        "ft_alembic_0.py",
        "ft_alembic_1.py",
        "ft_alembic_2.py",
        "ft_alembic_3.py",
        "ft_alembic_4.py",
        "ft_alembic_5.py",
        "ft_distillation_0.py",
        "ft_distillation_1.py",
        "ft_transmutation_0.py",
        "ft_transmutation_1.py",
        "ft_transmutation_2.py",
        "ft_kaboom_0.py",
        "ft_kaboom_1.py",
    ],
    "module07": [
        "ex0/__init__.py",
        "ex1/__init__.py",
        "ex2/__init__.py",
        "battle.py",
        "capacitor.py",
        "tournament.py",
    ],
    "module08": [
        "ex0/construct.py",
        "ex1/loading.py",
        "ex1/requirements.txt",
        "ex1/pyproject.toml",
        "ex2/oracle.py",
        "ex2/requirements.txt",
        "ex2/.env.example",
        "ex2/.gitignore",
    ],
    "module09": [
        "ex0/space_station.py",
        "ex1/alien_contact.py",
        "ex2/space_crew.py",
    ],
    "module10": [
        "ex0/lambda_spells.py",
        "ex1/higher_magic.py",
        "ex2/scope_mysteries.py",
        "ex3/functools_artifacts.py",
        "ex4/decorator_mastery.py",
    ],
}


@pytest.mark.parametrize("module", sorted(EXPECTED.keys()))
def test_module_structure(module):
    module_dir = ROOT / module
    assert module_dir.is_dir(), f"Dossier {module} absent"
    missing = [f for f in EXPECTED[module] if not (module_dir / f).exists()]
    assert not missing, f"Fichiers manquants dans {module}: {missing}"


def test_all_modules_present():
    for module in EXPECTED:
        assert (ROOT / module).is_dir(), f"Module {module} absent"
