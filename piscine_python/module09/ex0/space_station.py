"""
Module 09 - Exercice 0 : Space Station
Validation de données avec Pydantic v2 : BaseModel, Field (contraintes
numériques et de longueur), model_validate() et gestion des ValidationError.
"""

from datetime import datetime
from typing import Optional

from pydantic import BaseModel, Field, ValidationError


class SpaceStation(BaseModel):
    """Modèle de données d'une station spatiale avec contraintes."""

    station_id: str = Field(min_length=3, max_length=10)
    name: str = Field(min_length=1, max_length=50)
    crew_size: int = Field(ge=1, le=20)
    power_level: float = Field(ge=0.0, le=100.0)
    oxygen_level: float = Field(ge=0.0, le=100.0)
    last_maintenance: datetime
    is_operational: bool = Field(default=True)
    notes: Optional[str] = Field(max_length=200, default=None)


def create_station(**kwargs) -> SpaceStation:
    """Crée une station à partir d'un dictionnaire, lance ValidationError."""
    return SpaceStation.model_validate(kwargs)


def describe(station: SpaceStation) -> str:
    """Renvoie une description textuelle d'une station valide."""
    status = "Operational" if station.is_operational else "Inoperational"
    return (
        f"ID: {station.station_id}\nName: {station.name}\n"
        f"Crew: {station.crew_size} people\n"
        f"Power: {station.power_level}%\n"
        f"Oxygen: {station.oxygen_level}%\n"
        f"Status: {status}"
    )


def format_validation_error(exc: ValidationError) -> list[str]:
    """Extrait les messages d'erreur d'une ValidationError."""
    return [error["msg"].removeprefix("Value error, ") for error in exc.errors()]


def main() -> None:
    try:
        station = SpaceStation(
            station_id="ISS001",
            name="Orbital Research Hub",
            crew_size=6,
            power_level=87.5,
            oxygen_level=92.0,
            last_maintenance="2024-03-15T00:00:00",
            is_operational=True,
        )
        print("Space Station Data Validation")
        print("========================================")
        print("Valid station created:")
        print(describe(station))
    except ValidationError as exc:
        for message in format_validation_error(exc):
            print(message)

    print("\n========================================")
    try:
        SpaceStation(
            station_id="X",  # trop court
            name="Deep Space Observatory",
            crew_size=28,  # hors limite (>20)
            power_level=70.8,
            oxygen_level=88.1,
            last_maintenance="2023-08-24T00:00:00",
            is_operational=False,
            notes="System diagnostics required",
        )
    except ValidationError as exc:
        for message in format_validation_error(exc):
            print(message)


if __name__ == "__main__":
    main()
