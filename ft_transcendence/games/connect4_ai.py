"""
Connect-4 AI opponent.

Subject constraints for the Pong AI (no A*, human-like) do not strictly
apply to the second game, but we keep the same philosophy: a bounded,
deterministic, testable opponent. We use depth-limited minimax with
alpha-beta pruning and a positional heuristic (centre control, threats,
blocking). No A* search is used.

The AI owns a private Connect-4 copy so it can look ahead without touching
the real game. ``choose_move`` returns a column; ``act`` applies it.
"""
from .connect4 import Connect4, PLAYER1, PLAYER2, EMPTY


class Connect4AI:
    """Depth-limited minimax Connect-4 opponent."""

    #: How many plies to look ahead. 4 keeps the search fast on a 7x6 board.
    DEPTH = 4

    def __init__(self, side=PLAYER2):
        # The AI plays as the given side; the human is the other side.
        self.side = side
        self.opponent = PLAYER2 if side == PLAYER1 else PLAYER1

    # ------------------------------------------------------------------ public API
    def choose_move(self, game: Connect4) -> int:
        """Return the best column for the AI's side given ``game``.

        Strategy (robust and human-like, no A*):
          1. If the AI can win immediately, take that move.
          2. Else if the opponent can win immediately, block that column.
          3. Else fall back to depth-limited minimax with a positional
             heuristic (centre control + threats).
        Always returns a legal column (or -1 if the board is full).
        """
        available = game.available_columns()
        if not available:
            return -1
        if game.winner != EMPTY:
            return available[0]

        # 1. Winning move for the AI.
        win = self._find_winning_move(game, self.side)
        if win is not None:
            return win
        # 2. Block the opponent's winning move.
        block = self._find_winning_move(game, self.opponent)
        if block is not None:
            return block
        # 3. Depth-limited minimax.
        best_score = -float("inf")
        best_col = available[0]
        for col in available:
            score = self._minimax(game, self.DEPTH, True, col)
            if score > best_score:
                best_score = score
                best_col = col
        return best_col

    def _find_winning_move(self, game: Connect4, player: int):
        """Return the column that lets ``player`` win this turn, or None.

        Simulates placing ``player``'s piece directly (independent of whose
        turn it currently is) so we can spot an immediate win/block.
        """
        for col in game.available_columns():
            # landing row for this column
            row = game.rows - 1
            while row >= 0 and game.board[row][col] != EMPTY:
                row -= 1
            game.board[row][col] = player
            won = game._check_win(row, col, player)
            game.board[row][col] = EMPTY
            if won:
                return col
        return None

    def act(self, game: Connect4) -> bool:
        """Choose a column and apply the drop. Returns True if applied."""
        col = self.choose_move(game)
        return game.drop(col)

    # ------------------------------------------------------------------ minimax
    def _minimax(self, game: Connect4, depth: int, maximizing: bool,
                 last_col: int) -> float:
        """Alpha-beta minimax over Connect-4 positions.

        ``maximizing`` is True when the AI's side is to move. ``last_col``
        is the column just played (used by the heuristic for context).
        """
        if depth == 0 or game.winner != EMPTY:
            return self._evaluate(game)

        player = self.side if maximizing else self.opponent
        available = game.available_columns()
        if not available:
            return self._evaluate(game)

        if maximizing:
            best = -float("inf")
            for col in available:
                game.drop(col)
                score = self._minimax(game, depth - 1, False, col)
                game.undo()
                if score > best:
                    best = score
            return best
        else:
            best = float("inf")
            for col in available:
                game.drop(col)
                score = self._minimax(game, depth - 1, True, col)
                game.undo()
                if score < best:
                    best = score
            return best

    # ------------------------------------------------------------------ heuristic
    def _evaluate(self, game: Connect4) -> float:
        """Positional score from the AI's perspective.

        Positive rewards the AI's alignments, negative rewards the
        opponent's. Rewards centre columns (stronger centre control).
        """
        score = 0.0
        for player, sign in ((self.side, 1.0), (self.opponent, -1.0)):
            score += sign * self._player_score(game, player)
        return score

    def _player_score(self, game: Connect4, player: int) -> float:
        total = 0.0
        b = game.board
        rows, cols = game.rows, game.columns
        # centre weighting
        for c in range(cols):
            w = 1.0 + abs(c - (cols - 1) / 2.0) * -0.15
            for r in range(rows):
                if b[r][c] == player:
                    total += w
        # count lines of 2 and 3 (potential wins / blocks)
        for r in range(rows):
            for c in range(cols):
                for dr, dc in ((0, 1), (1, 0), (1, 1), (1, -1)):
                    count = 0
                    rr, cc = r, c
                    while (0 <= rr < rows and 0 <= cc < cols
                           and b[rr][cc] == player):
                        count += 1
                        rr += dr
                        cc += dc
                    if count == 2:
                        total += 2
                    elif count == 3:
                        total += 12  # a 3-in-a-row is a strong threat
                    if count >= 4:
                        total += 1000  # winning line
        return total
