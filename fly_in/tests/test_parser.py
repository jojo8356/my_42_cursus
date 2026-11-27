"""Parser tests: strict acceptance and rejection of every map variant.

Covers the subject's parser constraints: mandatory definitions, unique
names, valid zone types, positive capacities, duplicate detection, dash and
whitespace rules in names, metadata syntax, and reachability validation.
"""
from __future__ import annotations

from pathlib import Path

import pytest

from src.parser import MapError, MapParser

VALID_MAP = """\
nb_drones: 2

start_hub: start 0 0 [color=green]
hub: mid 1 0 [zone=priority color=blue max_drones=2]
end_hub: goal 2 0 [color=red]

connection: start-mid
connection: mid-goal
"""


def _write(tmp_path: Path, text: str) -> Path:
    """Write ``text`` to a temporary map file and return its path."""
    path = tmp_path / "map.txt"
    path.write_text(text, encoding="utf-8")
    return path


def test_parses_a_valid_map(parser: MapParser, tmp_path: Path) -> None:
    """A well-formed map parses into a complete network."""
    network = parser.parse(_write(tmp_path, VALID_MAP))
    assert network.drones == 2
    assert network.start == "start"
    assert network.end == "goal"
    assert set(network.zones) == {"start", "mid", "goal"}
    assert network.zones["mid"].kind == "priority"
    assert network.zones["mid"].capacity == 2


def test_nb_drones_must_come_first(parser: MapParser, tmp_path: Path) -> None:
    """The first definition must be nb_drones."""
    bad = "start_hub: s 0 0\nnb_drones: 1\nend_hub: e 1 0\n"
    with pytest.raises(MapError, match="first definition must be nb_drones"):
        parser.parse(_write(tmp_path, bad))


def test_missing_nb_drones(parser: MapParser, tmp_path: Path) -> None:
    """A map with no nb_drones (only comments) is rejected."""
    bad = "# nothing but a comment\n\n"
    with pytest.raises(MapError, match="missing nb_drones"):
        parser.parse(_write(tmp_path, bad))


def test_first_definition_must_be_nb_drones(parser: MapParser, tmp_path: Path) -> None:
    """Starting with a hub before nb_drones is rejected."""
    bad = "start_hub: s 0 0\nend_hub: e 1 0\n"
    with pytest.raises(MapError, match="first definition must be nb_drones"):
        parser.parse(_write(tmp_path, bad))


def test_duplicate_start_hub(parser: MapParser, tmp_path: Path) -> None:
    """A second start_hub is rejected."""
    bad = (
        "nb_drones: 1\n"
        "start_hub: s 0 0\nstart_hub: s2 1 0\n"
        "end_hub: e 2 0\n"
    )
    with pytest.raises(MapError, match="duplicate start_hub"):
        parser.parse(_write(tmp_path, bad))


def test_duplicate_zone(parser: MapParser, tmp_path: Path) -> None:
    """Re-declaring a hub name is rejected."""
    bad = (
        "nb_drones: 1\n"
        "start_hub: s 0 0\nhub: a 1 0\nhub: a 2 0\n"
        "end_hub: e 3 0\n"
    )
    with pytest.raises(MapError, match="duplicate zone"):
        parser.parse(_write(tmp_path, bad))


def test_zone_name_with_dash_rejected(parser: MapParser, tmp_path: Path) -> None:
    """Zone names may not contain dashes (they delimit connections)."""
    bad = (
        "nb_drones: 1\n"
        "start_hub: s 0 0\nhub: bad-name 1 0\n"
        "end_hub: e 2 0\n"
    )
    with pytest.raises(MapError, match="cannot contain dashes"):
        parser.parse(_write(tmp_path, bad))


def test_invalid_zone_type(parser: MapParser, tmp_path: Path) -> None:
    """An unknown zone type is rejected."""
    bad = (
        "nb_drones: 1\n"
        "start_hub: s 0 0\nhub: a 1 0 [zone=flying]\n"
        "end_hub: e 2 0\n"
    )
    with pytest.raises(MapError, match="invalid zone type"):
        parser.parse(_write(tmp_path, bad))


def test_valid_zone_kinds_accepted(parser: MapParser, tmp_path: Path) -> None:
    """The subject's normal/priority/restricted kinds are accepted on a route."""
    for kind in ("normal", "priority", "restricted"):
        text = (
            "nb_drones: 1\n"
            f"start_hub: s 0 0\nhub: a 1 0 [zone={kind}]\n"
            "end_hub: e 2 0\n"
            "connection: s-a\nconnection: a-e\n"
        )
        network = parser.parse(_write(tmp_path, text))
        assert network.zones["a"].kind == kind


def test_blocked_zone_off_route_accepted(parser: MapParser, tmp_path: Path) -> None:
    """A blocked zone kept off the main route parses fine."""
    text = (
        "nb_drones: 1\n"
        "start_hub: s 0 0\nhub: a 1 0\nhub: trap 0 1 [zone=blocked]\n"
        "end_hub: e 2 0\n"
        "connection: s-a\nconnection: a-trap\nconnection: a-e\n"
    )
    network = parser.parse(_write(tmp_path, text))
    assert network.zones["trap"].kind == "blocked"


def test_start_hub_cannot_be_blocked(parser: MapParser, tmp_path: Path) -> None:
    """The start zone may not be blocked."""
    bad = (
        "nb_drones: 1\n"
        "start_hub: s 0 0 [zone=blocked]\n"
        "end_hub: e 1 0\n"
    )
    with pytest.raises(MapError, match="cannot be blocked"):
        parser.parse(_write(tmp_path, bad))


def test_non_positive_capacity_rejected(parser: MapParser, tmp_path: Path) -> None:
    """max_drones and max_link_capacity must be positive integers."""
    zone_bad = (
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0 [max_drones=0]\n"
        "end_hub: e 2 0\nconnection: s-a\nconnection: a-e\n"
    )
    with pytest.raises(MapError, match="positive integer"):
        parser.parse(_write(tmp_path, zone_bad))

    link_bad = (
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0\nend_hub: e 2 0\n"
        "connection: s-a [max_link_capacity=-2]\nconnection: a-e\n"
    )
    with pytest.raises(MapError, match="positive integer"):
        parser.parse(_write(tmp_path, link_bad))


def test_duplicate_connection(parser: MapParser, tmp_path: Path) -> None:
    """A connection declared twice (a-b and b-a) is rejected."""
    bad = (
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0\nend_hub: e 2 0\n"
        "connection: s-a\nconnection: a-s\nconnection: a-e\n"
    )
    with pytest.raises(MapError, match="duplicate connection"):
        parser.parse(_write(tmp_path, bad))


def test_connection_to_undefined_zone(parser: MapParser, tmp_path: Path) -> None:
    """A connection referencing an unknown zone is rejected."""
    bad = (
        "nb_drones: 1\nstart_hub: s 0 0\nend_hub: e 2 0\n"
        "connection: s-ghost\n"
    )
    with pytest.raises(MapError, match="undefined zone"):
        parser.parse(_write(tmp_path, bad))


def test_connection_before_zone(parser: MapParser, tmp_path: Path) -> None:
    """Connections must reference previously defined zones."""
    bad = (
        "nb_drones: 1\nstart_hub: s 0 0\nend_hub: e 2 0\n"
        "connection: ghost-e\n"
    )
    with pytest.raises(MapError, match="undefined zone"):
        parser.parse(_write(tmp_path, bad))


def test_malformed_metadata(parser: MapParser, tmp_path: Path) -> None:
    """A malformed metadata block is rejected."""
    bad = (
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0 [zone=normal\n"
        "end_hub: e 2 0\nconnection: s-a\nconnection: a-e\n"
    )
    with pytest.raises(MapError, match="malformed metadata"):
        parser.parse(_write(tmp_path, bad))


def test_repeated_metadata(parser: MapParser, tmp_path: Path) -> None:
    """Repeating a metadata key is rejected."""
    bad = (
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0 [color=red color=blue]\n"
        "end_hub: e 2 0\nconnection: s-a\nconnection: a-e\n"
    )
    with pytest.raises(MapError, match="repeated metadata"):
        parser.parse(_write(tmp_path, bad))


def test_unknown_metadata(parser: MapParser, tmp_path: Path) -> None:
    """An unknown metadata key is rejected."""
    bad = (
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0 [speed=fast]\n"
        "end_hub: e 2 0\nconnection: s-a\nconnection: a-e\n"
    )
    with pytest.raises(MapError, match="unknown zone metadata"):
        parser.parse(_write(tmp_path, bad))


def test_unreachable_end_rejected(parser: MapParser, tmp_path: Path) -> None:
    """An end zone with no path from the start is rejected."""
    bad = (
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0\nend_hub: e 2 0\n"
        "connection: s-a\n"
    )
    with pytest.raises(MapError, match="no accessible route"):
        parser.parse(_write(tmp_path, bad))


def test_blocked_zone_does_not_block_route(parser: MapParser, tmp_path: Path) -> None:
    """A blocked branch is ignored while the real route stays valid."""
    text = (
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0\nhub: trap 0 1 [zone=blocked]\n"
        "end_hub: e 2 0\nconnection: s-a\nconnection: a-trap\nconnection: a-e\n"
    )
    network = parser.parse(_write(tmp_path, text))
    assert network.zones["trap"].kind == "blocked"
    assert network.end in {"e"}


def test_signed_coordinates_accepted(parser: MapParser, tmp_path: Path) -> None:
    """Coordinates may carry a sign and stay integer."""
    text = (
        "nb_drones: 1\nstart_hub: s -5 +3\nhub: a 2 -7\nend_hub: e 0 0\n"
        "connection: s-a\nconnection: a-e\n"
    )
    network = parser.parse(_write(tmp_path, text))
    assert network.zones["s"].x == -5
    assert network.zones["a"].y == -7


def test_comments_and_blank_lines_ignored(parser: MapParser, tmp_path: Path) -> None:
    """Comments and blank lines do not affect parsing."""
    text = (
        "# a header comment\n\nnb_drones: 1\n\n"
        "start_hub: s 0 0 # inline comment\n"
        "hub: a 1 0\n"
        "end_hub: e 2 0\n"
        "connection: s-a\nconnection: a-e\n"
    )
    network = parser.parse(_write(tmp_path, text))
    assert network.drones == 1
    assert network.start == "s"


def test_error_message_contains_line_number(parser: MapParser, tmp_path: Path) -> None:
    """Rejections report the offending line number."""
    bad = (
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0\n"
        "connection: ghost-e\n"
    )
    with pytest.raises(MapError, match=r"line 4"):
        parser.parse(_write(tmp_path, bad))


def test_self_connection_accepted(parser: MapParser, tmp_path: Path) -> None:
    """The subject does not forbid a zone connecting to itself."""
    text = (
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0\nend_hub: e 2 0\n"
        "connection: s-a\nconnection: a-a\nconnection: a-e\n"
    )
    network = parser.parse(_write(tmp_path, text))
    keys = [link.key for _, link in network.neighbors["a"]]
    assert ("a", "a") in keys
