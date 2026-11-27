"""Colored terminal rendering of the simulation movement log.

Each movement token is painted with the color declared on its destination
zone (a small ANSI palette).  Colors are cosmetic: the underlying text
remains exactly the subject output format, so output can be redirected to a
file without any escape sequences when the ``--plain`` flag is used.
"""
from __future__ import annotations

from src.models import Network, Step
from src.simulation import build_turns, total_turns

# ANSI foreground colors keyed by the map color name (case-insensitive).
_ANSI = {
    "black": "30",
    "red": "31",
    "green": "32",
    "yellow": "33",
    "blue": "34",
    "magenta": "35",
    "cyan": "36",
    "white": "37",
    "gray": "90",
    "orange": "38;5;208",
    "purple": "38;5;93",
    "lime": "38;5;100",
    "brown": "38;5;124",
    "gold": "38;5;179",
}
_RESET = "\033[0m"


class MovementRenderer:
    """Print one space-separated movement line per simulation turn."""

    def __init__(self, network: Network, color: bool = True) -> None:
        """Store the network and whether ANSI colors should be applied."""
        self.network = network
        self.color = color

    def _paint(self, token: str, zone_color: str) -> str:
        """Return the token, optionally wrapped in an ANSI color."""
        if not self.color:
            return token
        code = _ANSI.get(zone_color.lower())
        if code is None:
            return token
        return f"\033[{code}m{token}{_RESET}"

    def render(self, routes: list[list[Step]]) -> str:
        """Return the full movement log as a single string."""
        turns = build_turns(routes)
        lines: list[str] = []
        for turn in range(1, total_turns(turns) + 1):
            moves = sorted(turns.get(turn, []))
            tokens: list[str] = []
            for drone, target in moves:
                zone = self.network.zones.get(target)
                color = zone.color if zone is not None else "cyan"
                tokens.append(self._paint(f"D{drone}-{target}", color))
            lines.append(" ".join(tokens))
        return "\n".join(lines)

    def print_run(self, routes: list[list[Step]]) -> None:
        """Print the movement log to stdout, one turn per line."""
        print(self.render(routes), flush=True)
