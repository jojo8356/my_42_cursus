"""
Comprehensive tests for the tournament bracket and matchmaking
(tournaments/bracket.py).

Covers: bracket structure, byes, seeding, round-robin, matchmaking queue.
"""
from django.test import SimpleTestCase

from tournaments.bracket import (
    build_bracket, assign_seeding, bracket_rounds, MatchmakingQueue,
    generate_round_robin,
)


class BracketTests(SimpleTestCase):
    def test_requires_two_players(self):
        with self.assertRaises(ValueError):
            build_bracket(1)

    def test_bracket_power_of_two(self):
        root = build_bracket(8)
        leaves = []
        def collect(n):
            if n.is_leaf:
                leaves.append(n)
            else:
                for c in n.children:
                    collect(c)
        collect(root)
        self.assertEqual(len(leaves), 8)

    def test_bracket_pads_to_power_of_two(self):
        root = build_bracket(5)
        leaves = []
        def collect(n):
            if n.is_leaf:
                leaves.append(n)
            else:
                for c in n.children:
                    collect(c)
        collect(root)
        # padded to 8
        self.assertEqual(len(leaves), 8)
        byes = sum(1 for n in leaves if n.player == -1)
        self.assertEqual(byes, 3)

    def test_assign_seeding(self):
        root = build_bracket(4)
        assign_seeding(root, [0, 1, 2, 3])
        leaves = []
        def collect(n):
            if n.is_leaf:
                leaves.append(n)
            else:
                for c in n.children:
                    collect(c)
        collect(root)
        real = [n.player for n in leaves if n.player >= 0]
        # standard seeding: 0 vs 3, 1 vs 2
        self.assertEqual(set(real), {0, 1, 2, 3})

    def test_bracket_rounds_structure(self):
        root = build_bracket(4)
        assign_seeding(root, [0, 1, 2, 3])
        rounds = bracket_rounds(root)
        # 4 players => 2 rounds (semifinals + final)
        self.assertEqual(len(rounds), 2)
        self.assertEqual(len(rounds[0]), 2)  # 2 semifinals
        self.assertEqual(len(rounds[1]), 1)  # 1 final

    def test_round_robin_even(self):
        rounds = generate_round_robin([0, 1, 2, 3])
        self.assertEqual(len(rounds), 3)  # n-1 rounds
        for r in rounds:
            self.assertEqual(len(r), 2)  # n/2 matches

    def test_round_robin_odd(self):
        # 3 players (odd): padded to 4 with a bye => 3 rounds, 1 match each
        rounds = generate_round_robin([0, 1, 2])
        self.assertEqual(len(rounds), 3)
        for r in rounds:
            self.assertEqual(len(r), 1)  # one match per round (one bye)

    def test_round_robin_covers_all_pairs(self):
        players = [0, 1, 2, 3]
        rounds = generate_round_robin(players)
        seen = set()
        for r in rounds:
            for a, b in r:
                seen.add((min(a, b), max(a, b)))
        # all 6 unique pairs
        self.assertEqual(len(seen), 6)


class MatchmakingQueueTests(SimpleTestCase):
    def test_no_match_with_single_player(self):
        q = MatchmakingQueue()
        self.assertIsNone(q.join(1))
        self.assertEqual(q.queue_size(), 1)

    def test_match_created_with_two_players(self):
        q = MatchmakingQueue()
        q.join(1)
        match = q.join(2)
        self.assertIsNotNone(match)
        self.assertEqual(match, (1, 2))

    def test_queue_drains_on_match(self):
        q = MatchmakingQueue()
        q.join(1)
        q.join(2)
        self.assertEqual(q.queue_size(), 0)

    def test_duplicate_join_ignored(self):
        q = MatchmakingQueue()
        q.join(1)
        self.assertIsNone(q.join(1))
        self.assertEqual(q.queue_size(), 1)

    def test_leave_queue(self):
        q = MatchmakingQueue()
        q.join(1)
        q.join(2)
        q.join(3)
        q.leave(3)
        self.assertEqual(q.queue_size(), 0)  # 1 and 2 were matched

    def test_next_match(self):
        q = MatchmakingQueue()
        self.assertIsNone(q.next_match())
        q.join(1)
        q.join(2)
        self.assertEqual(q.next_match(), (1, 2))

    def test_pop_match(self):
        q = MatchmakingQueue()
        q.join(1)
        q.join(2)
        self.assertEqual(q.pop_match(), (1, 2))
        self.assertIsNone(q.pop_match())

    def test_multiple_matches(self):
        q = MatchmakingQueue()
        q.join(1); q.join(2)
        q.join(3); q.join(4)
        self.assertEqual(len(q.matches), 2)
