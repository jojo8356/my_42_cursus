"""
Tournament bracket generation and matchmaking.

Implements a single-elimination bracket:
  - players are seeded into rounds
  - each round pairs adjacent players
  - winners advance; the bracket clearly shows who plays whom and the order
  - matchmaking: a queue of waiting players is paired to announce the next fight
"""
from dataclasses import dataclass, field


@dataclass
class BracketNode:
    """A node in the single-elimination bracket tree."""
    player: int = -1          # player index, -1 = open slot / bye
    winner: int = -1
    children: list = field(default_factory=list)

    @property
    def is_leaf(self):
        return not self.children

    def is_complete(self):
        if self.is_leaf:
            return self.player >= 0
        return all(c.is_complete() for c in self.children)


def build_bracket(num_players):
    """Build a balanced single-elimination bracket for ``num_players``.

    Returns the root node. Number of players is padded to the next power of
    two with byes (player = -1).
    """
    if num_players < 2:
        raise ValueError("A tournament needs at least 2 players")

    # pad to next power of two
    size = 1
    while size < num_players:
        size *= 2

    leaves = [BracketNode(player=i if i < num_players else -1) for i in range(size)]
    # build the tree bottom-up
    level = leaves
    while len(level) > 1:
        next_level = []
        for i in range(0, len(level), 2):
            left, right = level[i], level[i + 1]
            parent = BracketNode(children=[left, right])
            next_level.append(parent)
        level = next_level
    return level[0]


def assign_seeding(bracket_root, seeding):
    """Place players into the bracket according to a seeding order.

    ``seeding`` is a list of player indices in seed order
    (seed 0 = top seed). Byes stay as -1.
    """
    leaves = []

    def collect(node):
        if node.is_leaf:
            leaves.append(node)
        else:
            for c in node.children:
                collect(c)

    collect(bracket_root)
    # number of real slots
    real = sum(1 for n in leaves if n.player >= 0)
    ordered = [s for s in seeding if s is not None]
    # standard seeding: 1st vs last, 2nd vs second-last, ...
    paired = []
    left, right = 0, len(ordered) - 1
    while left <= right:
        paired.append(ordered[left])
        if left != right:
            paired.append(ordered[right])
        left += 1
        right -= 1
    # fill real leaves in order
    idx = 0
    for leaf in leaves:
        if leaf.player >= 0 and idx < len(paired):
            leaf.player = paired[idx]
            idx += 1
    return bracket_root


def bracket_rounds(bracket_root):
    """Return the bracket as a list of rounds (each a list of matches).

    A round is a set of matches played at the same bracket level, from the
    first round (leaf parents) up to the final (root). Each match is a tuple
    (player_a, player_b) where -1 is a bye.
    """
    # Compute each node's level = max subtree height (leaves = 0).
    def level(node):
        if node.is_leaf:
            return 0
        return 1 + max(level(c) for c in node.children)

    # Collect match nodes (non-leaf) grouped by their level.
    matches_by_level = {}

    def collect(node):
        if node.is_leaf:
            return
        lvl = level(node)
        matches_by_level.setdefault(lvl, []).append(node)
        for c in node.children:
            collect(c)

    collect(bracket_root)
    # level 1 = first round, level 2 = next, etc.
    rounds = []
    for lvl in sorted(matches_by_level.keys()):
        nodes = matches_by_level[lvl]
        matches = []
        for node in nodes:
            a = node.children[0] if node.children[0] else None
            b = node.children[1] if len(node.children) > 1 and node.children[1] else None
            matches.append((
                a.player if a else -1,
                b.player if b else -1,
            ))
        rounds.append(matches)
    return rounds


def advance_winner(match_a_winner, match_b_winner):
    """Return the winner of a match given two sub-winners.

    A bye (-1) means the other player advances automatically.
    """
    if match_a_winner == -1:
        return match_b_winner
    if match_b_winner == -1:
        return match_a_winner
    return match_a_winner  # the caller decides the actual winner


class MatchmakingQueue:
    """Matchmaking system: players join a queue and are paired up.

    When two players are waiting, a match is created and announced.
    """

    def __init__(self):
        self.queue = []
        self.matches = []  # list of (player_a, player_b) pending matches

    def join(self, player):
        """A player joins the queue. Returns the announced match or None."""
        if player in self.queue:
            return None
        self.queue.append(player)
        if len(self.queue) >= 2:
            a = self.queue.pop(0)
            b = self.queue.pop(0)
            match = (a, b)
            self.matches.append(match)
            return match
        return None

    def leave(self, player):
        if player in self.queue:
            self.queue.remove(player)

    def next_match(self):
        """Return the next pending match (the announced next fight)."""
        if not self.matches:
            return None
        return self.matches[0]

    def pop_match(self):
        if not self.matches:
            return None
        return self.matches.pop(0)

    def queue_size(self):
        return len(self.queue)


def generate_round_robin(players):
    """Generate a round-robin schedule (used for the tournament order display).

    Returns a list of rounds, each a list of (a, b) pairs.
    """
    if len(players) < 2:
        return []
    if len(players) % 2 == 1:
        players = players + [None]
    n = len(players)
    rounds = []
    arr = list(players)
    for _ in range(n - 1):
        round_matches = []
        for i in range(n // 2):
            a = arr[i]
            b = arr[n - 1 - i]
            if a is not None and b is not None:
                round_matches.append((a, b))
        rounds.append(round_matches)
        # rotate: keep first fixed, rotate the rest
        arr = [arr[0]] + [arr[-1]] + arr[1:-1]
    return rounds
