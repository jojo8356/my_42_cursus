"""
Module 09 - Exercice 1 : Alien Contact
Pydantic v2 : Enum pour les valeurs contrôlées et @model_validator
(mode='after') pour des règles métier inter-champs.
"""

from datetime import datetime
from enum import Enum
from typing import Optional

from pydantic import BaseModel, Field, ValidationError, model_validator


class ContactType(str, Enum):
    """Type de contact extraterrestre."""

    RADIO = "radio"
    VISUAL = "visual"
    PHYSICAL = "physical"
    TELEPATHIC = "telepathic"


class AlienContact(BaseModel):
    """Modèle de données d'un contact alien avec validators métier."""

    contact_id: str = Field(min_length=5, max_length=15)
    timestamp: datetime
    location: str = Field(min_length=3, max_length=100)
    contact_type: ContactType
    signal_strength: float = Field(ge=0.0, le=10.0)
    duration_minutes: int = Field(ge=1, le=1440)
    witness_count: int = Field(ge=1, le=100)
    message_received: Optional[str] = Field(max_length=500, default=None)
    is_verified: bool = Field(default=False)

    @model_validator(mode="after")
    def id_validator(self) -> "AlienContact":
        """L'identifiant doit commencer par 'AC'."""
        if not self.contact_id.startswith("AC"):
            raise ValueError(f"All id's must start with 'AC'. Error: {self.contact_id}")
        return self

    @model_validator(mode="after")
    def contact_validator(self) -> "AlienContact":
        """Règles métier selon le type de contact."""
        if self.contact_type == ContactType.PHYSICAL and not self.is_verified:
            raise ValueError("Physical contacts must be verified")
        if self.contact_type == ContactType.TELEPATHIC and self.witness_count < 3:
            raise ValueError("Telepathic contacts must have at least 3 witnesses")
        return self

    @model_validator(mode="after")
    def signal_validator(self) -> "AlienContact":
        """Un signal fort doit comporter un message reçu."""
        if self.signal_strength > 7.0 and not self.message_received:
            raise ValueError("Strong signals must include a received messages")
        return self


def format_validation_error(exc: ValidationError) -> list[str]:
    """Extrait les messages d'erreur d'une ValidationError."""
    return [error["msg"].removeprefix("Value error, ") for error in exc.errors()]


def main() -> None:
    valid_data = {
        "contact_id": "AC12345",
        "timestamp": "2024-05-01T12:00:00",
        "location": "Sector 7G",
        "contact_type": ContactType.RADIO,
        "signal_strength": 6.0,
        "duration_minutes": 30,
        "witness_count": 2,
        "message_received": "Hello there",
        "is_verified": True,
    }
    try:
        contact = AlienContact.model_validate(valid_data)
        print("Valid alien contact created:")
        print(contact)
    except ValidationError as exc:
        for message in format_validation_error(exc):
            print(message)

    # Cas invalides démontrant les validators métier
    for label, bad in {
        "bad_id": {**valid_data, "contact_id": "XY12345"},
        "physical_unverified": {
            **valid_data,
            "contact_id": "AC12346",
            "contact_type": ContactType.PHYSICAL,
            "is_verified": False,
        },
        "telepathic_no_witness": {
            **valid_data,
            "contact_id": "AC12347",
            "contact_type": ContactType.TELEPATHIC,
            "witness_count": 1,
        },
        "strong_signal_no_message": {
            **valid_data,
            "contact_id": "AC12348",
            "signal_strength": 9.0,
            "message_received": None,
        },
    }.items():
        try:
            AlienContact.model_validate(bad)
            print(f"{label}: unexpectedly valid")
        except ValidationError as exc:
            print(f"{label}: {format_validation_error(exc)[0]}")


if __name__ == "__main__":
    main()
