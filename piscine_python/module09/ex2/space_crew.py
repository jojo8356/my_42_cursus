"""
Module 09 - Exercice 2 : Space Crew
Pydantic v2 : modèles imbriqués (List[CrewMember]), validators de
collection et règles d'expérience / de leadership.
"""

from datetime import datetime
from enum import Enum
from typing import List

from pydantic import BaseModel, Field, ValidationError, model_validator


class Rank(str, Enum):
    """Grade d'un membre d'équipage."""

    CADET = "cadet"
    OFFICER = "officer"
    LIEUTENANT = "lieutenant"
    CAPTAIN = "captain"
    COMMANDER = "commander"


class CrewMember(BaseModel):
    """Un membre d'équipage."""

    member_id: str = Field(min_length=3, max_length=10)
    name: str = Field(min_length=2, max_length=50)
    rank: Rank
    age: int = Field(ge=18, le=80)
    specialization: str = Field(min_length=3, max_length=30)
    years_experience: int = Field(ge=0, le=50)
    is_active: bool = Field(default=True)


class SpaceMission(BaseModel):
    """Une mission spatiale avec son équipage (modèle imbriqué)."""

    mission_id: str = Field(min_length=5, max_length=15)
    mission_name: str = Field(min_length=3, max_length=300)
    destination: str = Field(min_length=3, max_length=50)
    launch_date: datetime
    duration_days: int = Field(ge=1, le=3650)
    crew: List[CrewMember] = Field(min_length=1, max_length=12)
    mission_status: str = Field(default="planned")
    budget_millions: float = Field(ge=1.0, le=10000.0)

    @model_validator(mode="after")
    def id_validator(self) -> "SpaceMission":
        """L'identifiant de mission doit commencer par 'M'."""
        if not self.mission_id.startswith("M"):
            raise ValueError(f"All id's must start with 'M'. Error: {self.mission_id}")
        return self

    @model_validator(mode="after")
    def leader_checker(self) -> "SpaceMission":
        """La mission doit avoir au moins un Captain ou Commander."""
        for leader in self.crew:
            if leader.rank in (Rank.CAPTAIN, Rank.COMMANDER):
                return self
        raise ValueError("Mission must have at least one Commander or Captain")

    @model_validator(mode="after")
    def experience_crew(self) -> "SpaceMission":
        """Mission longue : au moins 50 % de l'équipe avec 5+ ans d'expérience."""
        if self.duration_days > 365:
            experienced = sum(1 for member in self.crew if member.years_experience >= 5)
            if len(self.crew) // 2 <= experienced:
                return self
            raise ValueError("A team must have at least 50% of team members with 5+ years experience")
        return self

    @model_validator(mode="after")
    def active_checker(self) -> "SpaceMission":
        """Tous les membres doivent être actifs."""
        if len(self.crew) != sum(1 for member in self.crew if member.is_active):
            raise ValueError("All members must be active")
        return self


def format_validation_error(exc: ValidationError) -> list[str]:
    """Extrait les messages d'erreur d'une ValidationError."""
    return [error["msg"].removeprefix("Value error, ") for error in exc.errors()]


def main() -> None:
    crew = [
        CrewMember(
            member_id="CM01",
            name="Aria Vance",
            rank=Rank.COMMANDER,
            age=45,
            specialization="Navigation",
            years_experience=15,
        ),
        CrewMember(
            member_id="CM02",
            name="Bo Reynolds",
            rank=Rank.OFFICER,
            age=38,
            specialization="Engineering",
            years_experience=8,
        ),
    ]
    try:
        mission = SpaceMission(
            mission_id="M2024X",
            mission_name="Red Planet Survey",
            destination="Mars",
            launch_date="2024-07-01T00:00:00",
            duration_days=400,
            crew=crew,
            budget_millions=250.0,
        )
        print("Valid mission created:")
        print(f"{mission.mission_id} - {mission.mission_name}")
        print(f"Crew size: {len(mission.crew)}")
    except ValidationError as exc:
        for message in format_validation_error(exc):
            print(message)

    # Cas invalides
    no_leader = [
        CrewMember(
            member_id="CM03",
            name="Cara Doe",
            rank=Rank.CADET,
            age=25,
            specialization="Comms",
            years_experience=2,
        )
    ]
    for label, bad in {
        "no_leader": {
            "mission_id": "M2025A",
            "mission_name": "Short Flight",
            "destination": "Orbit",
            "launch_date": "2024-08-01T00:00:00",
            "duration_days": 30,
            "crew": no_leader,
            "budget_millions": 10.0,
        },
        "bad_id": {
            "mission_id": "X2025A",
            "mission_name": "Short Flight",
            "destination": "Orbit",
            "launch_date": "2024-08-01T00:00:00",
            "duration_days": 30,
            "crew": crew,
            "budget_millions": 10.0,
        },
    }.items():
        try:
            SpaceMission.model_validate(bad)
            print(f"{label}: unexpectedly valid")
        except ValidationError as exc:
            print(f"{label}: {format_validation_error(exc)[0]}")


if __name__ == "__main__":
    main()
