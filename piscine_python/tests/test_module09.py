"""
Tests complets - Module 09 (Pydantic v2 : validation de données).
Couverture : contraintes de Field, model_validate, Enum, validators métier
inter-champs, modèles imbriqués et gestion des erreurs.
"""

import datetime

import pytest
from pydantic import ValidationError

from conftest import run_script


# ---------------------------------------------------------------- ex0
class TestSpaceStation:
    def test_valid_station(self, module09_ex0):
        ss = module09_ex0.SpaceStation.model_validate(
            {
                "station_id": "ISS001",
                "name": "Orbital Hub",
                "crew_size": 6,
                "power_level": 87.5,
                "oxygen_level": 92.0,
                "last_maintenance": "2024-03-15T00:00:00",
            }
        )
        assert ss.station_id == "ISS001"
        assert ss.is_operational is True
        assert ss.notes is None
        assert isinstance(ss.last_maintenance, datetime.datetime)

    def test_crew_size_bounds(self, module09_ex0):
        with pytest.raises(ValidationError):
            module09_ex0.SpaceStation.model_validate(
                {
                    "station_id": "ISS001",
                    "name": "Orbital Hub",
                    "crew_size": 0,
                    "power_level": 50.0,
                    "oxygen_level": 50.0,
                    "last_maintenance": "2024-03-15T00:00:00",
                }
            )
        with pytest.raises(ValidationError):
            module09_ex0.SpaceStation.model_validate(
                {
                    "station_id": "ISS001",
                    "name": "Orbital Hub",
                    "crew_size": 21,
                    "power_level": 50.0,
                    "oxygen_level": 50.0,
                    "last_maintenance": "2024-03-15T00:00:00",
                }
            )

    def test_station_id_length(self, module09_ex0):
        base = {
            "name": "Orbital Hub",
            "crew_size": 6,
            "power_level": 50.0,
            "oxygen_level": 50.0,
            "last_maintenance": "2024-03-15T00:00:00",
        }
        with pytest.raises(ValidationError):
            module09_ex0.SpaceStation.model_validate({**base, "station_id": "AB"})
        with pytest.raises(ValidationError):
            module09_ex0.SpaceStation.model_validate({**base, "station_id": "ABCDEFGHIJK"})

    def test_power_oxygen_bounds(self, module09_ex0):
        base = {
            "station_id": "ISS001",
            "name": "Orbital Hub",
            "crew_size": 6,
            "last_maintenance": "2024-03-15T00:00:00",
        }
        with pytest.raises(ValidationError):
            module09_ex0.SpaceStation.model_validate({**base, "power_level": 101.0, "oxygen_level": 50.0})
        with pytest.raises(ValidationError):
            module09_ex0.SpaceStation.model_validate({**base, "power_level": 50.0, "oxygen_level": -1.0})

    def test_notes_length(self, module09_ex0):
        base = {
            "station_id": "ISS001",
            "name": "Orbital Hub",
            "crew_size": 6,
            "power_level": 50.0,
            "oxygen_level": 50.0,
            "last_maintenance": "2024-03-15T00:00:00",
        }
        with pytest.raises(ValidationError):
            module09_ex0.SpaceStation.model_validate({**base, "notes": "x" * 201})

    def test_describe_and_error_formatting(self, module09_ex0):
        ss = module09_ex0.SpaceStation.model_validate(
            {
                "station_id": "ISS001",
                "name": "Orbital Hub",
                "crew_size": 6,
                "power_level": 87.5,
                "oxygen_level": 92.0,
                "last_maintenance": "2024-03-15T00:00:00",
                "is_operational": False,
            }
        )
        text = module09_ex0.describe(ss)
        assert "Inoperational" in text
        assert "Crew: 6 people" in text

    def test_script_runs(self, module09_ex0):
        code, out, err = run_script("module09/ex0/space_station.py")
        assert code == 0, err
        assert "Space Station Data Validation" in out
        assert "Valid station created" in out


# ---------------------------------------------------------------- ex1
class TestAlienContact:
    def _valid(self):
        return {
            "contact_id": "AC12345",
            "timestamp": "2024-05-01T12:00:00",
            "location": "Sector 7G",
            "contact_type": "radio",
            "signal_strength": 6.0,
            "duration_minutes": 30,
            "witness_count": 2,
            "message_received": "Hello",
            "is_verified": True,
        }

    def test_valid_contact(self, module09_ex1):
        c = module09_ex1.AlienContact.model_validate(self._valid())
        assert c.contact_type == module09_ex1.ContactType.RADIO
        assert c.is_verified is True

    def test_id_must_start_ac(self, module09_ex1):
        bad = self._valid()
        bad["contact_id"] = "XY12345"
        with pytest.raises(ValidationError, match="must start with 'AC'"):
            module09_ex1.AlienContact.model_validate(bad)

    def test_physical_must_be_verified(self, module09_ex1):
        bad = self._valid()
        bad.update(contact_id="AC12346", contact_type="physical", is_verified=False)
        with pytest.raises(ValidationError, match="Physical contacts must be verified"):
            module09_ex1.AlienContact.model_validate(bad)

    def test_physical_verified_ok(self, module09_ex1):
        ok = self._valid()
        ok.update(contact_id="AC12347", contact_type="physical", is_verified=True)
        contact = module09_ex1.AlienContact.model_validate(ok)
        assert contact.contact_type == module09_ex1.ContactType.PHYSICAL

    def test_telepathic_requires_witnesses(self, module09_ex1):
        bad = self._valid()
        bad.update(contact_id="AC12348", contact_type="telepathic", witness_count=2)
        with pytest.raises(ValidationError, match="at least 3 witnesses"):
            module09_ex1.AlienContact.model_validate(bad)
        ok = self._valid()
        ok.update(contact_id="AC12349", contact_type="telepathic", witness_count=3)
        module09_ex1.AlienContact.model_validate(ok)

    def test_strong_signal_requires_message(self, module09_ex1):
        bad = self._valid()
        bad.update(contact_id="AC12350", signal_strength=9.0, message_received=None)
        with pytest.raises(ValidationError, match="received messages"):
            module09_ex1.AlienContact.model_validate(bad)

    def test_signal_strength_bounds(self, module09_ex1):
        bad = self._valid()
        bad.update(contact_id="AC12351", signal_strength=11.0)
        with pytest.raises(ValidationError):
            module09_ex1.AlienContact.model_validate(bad)

    def test_script_runs(self, module09_ex1):
        code, out, err = run_script("module09/ex1/alien_contact.py")
        assert code == 0, err
        assert "Valid alien contact created" in out
        assert "Physical contacts must be verified" in out


# ---------------------------------------------------------------- ex2
class TestSpaceCrew:
    def _crew(self, rank="commander", exp=15, active=True):
        return [
            {
                "member_id": "CM01",
                "name": "Aria Vance",
                "rank": rank,
                "age": 45,
                "specialization": "Navigation",
                "years_experience": exp,
                "is_active": active,
            }
        ]

    def _mission(self, crew=None, **overrides):
        data = {
            "mission_id": "M2024X",
            "mission_name": "Red Planet Survey",
            "destination": "Mars",
            "launch_date": "2024-07-01T00:00:00",
            "duration_days": 400,
            "crew": crew if crew is not None else self._crew(),
            "budget_millions": 250.0,
        }
        data.update(overrides)
        return data

    def test_valid_mission(self, module09_ex2):
        m = module09_ex2.SpaceMission.model_validate(self._mission())
        assert len(m.crew) == 1
        assert m.crew[0].rank == module09_ex2.Rank.COMMANDER

    def test_mission_id_must_start_m(self, module09_ex2):
        with pytest.raises(ValidationError, match="must start with 'M'"):
            module09_ex2.SpaceMission.model_validate(self._mission(mission_id="X2024X"))

    def test_needs_leader(self, module09_ex2):
        crew = self._crew(rank="cadet", exp=2)
        with pytest.raises(ValidationError, match="at least one Commander or Captain"):
            module09_ex2.SpaceMission.model_validate(self._mission(crew=crew))

    def test_long_mission_needs_experience(self, module09_ex2):
        # 2 membres, long vol (>365j), 0 avec 5+ ans -> échec (0 < 1)
        crew = [
            {"member_id": "CM01", "name": "A B", "rank": "captain", "age": 40,
             "specialization": "Nav", "years_experience": 1, "is_active": True},
            {"member_id": "CM02", "name": "C D", "rank": "officer", "age": 30,
             "specialization": "Eng", "years_experience": 1, "is_active": True},
        ]
        with pytest.raises(ValidationError, match="at least 50% of team members"):
            module09_ex2.SpaceMission.model_validate(self._mission(crew=crew, duration_days=400))

    def test_long_mission_with_experience_ok(self, module09_ex2):
        crew = [
            {"member_id": "CM01", "name": "A B", "rank": "captain", "age": 40,
             "specialization": "Nav", "years_experience": 6, "is_active": True},
            {"member_id": "CM02", "name": "C D", "rank": "officer", "age": 30,
             "specialization": "Eng", "years_experience": 2, "is_active": True},
        ]
        m = module09_ex2.SpaceMission.model_validate(self._mission(crew=crew, duration_days=400))
        assert m.duration_days == 400

    def test_all_members_must_be_active(self, module09_ex2):
        crew = self._crew(active=False)
        with pytest.raises(ValidationError, match="All members must be active"):
            module09_ex2.SpaceMission.model_validate(self._mission(crew=crew))

    def test_crew_size_bounds(self, module09_ex2):
        with pytest.raises(ValidationError):
            module09_ex2.SpaceMission.model_validate(self._mission(crew=[]))
        # 13 membres > max_length 12
        big = [
            {"member_id": f"CM{i}", "name": f"Membre {i}", "rank": "commander",
             "age": 40, "specialization": "Nav", "years_experience": 10, "is_active": True}
            for i in range(13)
        ]
        with pytest.raises(ValidationError):
            module09_ex2.SpaceMission.model_validate(self._mission(crew=big))

    def test_script_runs(self, module09_ex2):
        code, out, err = run_script("module09/ex2/space_crew.py")
        assert code == 0, err
        assert "Valid mission created" in out
        assert "at least one Commander or Captain" in out
