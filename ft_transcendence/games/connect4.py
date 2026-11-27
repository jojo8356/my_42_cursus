"""
Connect-4 (Puissance 4) game engine - pure, deterministic, fully testable.

Second game required by the subject (module 7: "Ajouter un autre jeu avec
historique utilisateur et matchmaking"). Rules:
  - 6 rows x 7 columns board
  - players take turns dropping a piece into a column; it falls to the
    lowest empty cell
  - the first player to align 4 of their pieces horizontally, vertically
    or diagonally wins
  - the board fills up -> draw

Mirroring the Pong engine, the state transitions are deterministic so every
rule can be unit-tested without a browser or a model.
"""
from dataclasses import dataclass, field

# Standard Connect-4 dimensions.
ROWS = 6
COLUMNS = 7
WIN_LENGTH = 4

# Cell values.
EMPTY = 0
PLAYER1 = 1  # left / red
PLAYER2 = 2  # right / yellow


@dataclass
class Connect4:
    """A deterministic Connect-4 game state."""
    rows: int = ROWS
    columns: int = COLUMNS
    # board[row][col]; row 0 is the TOP row, row (rows-1) is the BOTTOM.
    board: list = field(
        default_factory=lambda: [[EMPTY] * COLUMNS for _ in range(ROWS)]
    )
    current: int = PLAYER1  # whose turn it is
    winner: int = EMPTY     # EMPTY | PLAYER1 | PLAYER2 | 3 (draw)
    move_count: int = 0
    # internal history stack so the AI can undo its look-ahead moves
    _history: list = field(default_factory=list)

    # ------------------------------------------------------------------ lifecycle
    def reset(self):
        """Clear the board and restart with player 1 to move."""
        self.board = [[EMPTY] * self.columns for _ in range(self.rows)]
        self.current = PLAYER1
        self.winner = EMPTY
        self.move_count = 0
        self._history = []

    # ------------------------------------------------------------------ moves
    def is_valid_move(self, col) -> bool:
        """A column accepts a drop if it is in range and not full."""
        if not isinstance(col, int) or col < 0 or col >= self.columns:
            return False
        if not self.board or not self.board[0]:
            return False
        return self.board[0][col] == EMPTY

    def available_columns(self) -> list:
        """Return the columns that still accept a drop."""
        return [c for c in range(self.columns) if self.is_valid_move(c)]

    def is_full(self) -> bool:
        return not self.available_columns()

    def drop(self, col) -> bool:
        """Drop the current player's piece into ``col``.

        Returns True if the move was legal and applied, False otherwise.
        After a legal move the turn switches, and the winner is resolved.
        """
        if not self.is_valid_move(col):
            return False
        # find the lowest empty row (fall to the bottom). Row 0 is the TOP,
        # row (rows-1) is the BOTTOM, so we scan from the bottom upward.
        row = self.rows - 1
        while row >= 0 and self.board[row][col] != EMPTY:
            row -= 1
        # remember state for undo (used by the AI look-ahead)
        self._history.append((row, col, self.current, self.winner,
                              self.move_count))
        self.board[row][col] = self.current
        self.move_count += 1
        # resolve the game
        if self._check_win(row, col, self.current):
            self.winner = self.current
        elif self.is_full():
            self.winner = 3  # draw
        else:
            self.current = PLAYER2 if self.current == PLAYER1 else PLAYER1
        return True

    def undo(self) -> bool:
        """Revert the last move (used by the AI to restore its look-ahead).

        Returns True if a move was reverted.
        """
        if not self._history:
            return False
        row, col, current, winner, move_count = self._history.pop()
        self.board[row][col] = EMPTY
        self.current = current
        self.winner = winner
        self.move_count = move_count
        return True

    # ------------------------------------------------------------------ win detection
    def _check_win(self, row, col, player) -> bool:
        """Check if placing ``player`` at (row, col) completes a line of 4."""
        directions = ((0, 1), (1, 0), (1, 1), (1, -1))
        for dr, dc in directions:
            count = 1
            # forward
            r, c = row + dr, col + dc
            while (0 <= r < self.rows and 0 <= c < self.columns
                   and self.board[r][c] == player):
                count += 1
                r += dr
                c += dc
            # backward
            r, c = row - dr, col - dc
            while (0 <= r < self.rows and 0 <= c < self.columns
                   and self.board[r][c] == player):
                count += 1
                r -= dr
                c -= dc
            if count >= WIN_LENGTH:
                return True
        return False

    def winning_line(self, player=None):
        """Return the winning line as a list of (row, col) if there is one.

        ``player`` defaults to the resolved winner. Returns None if no line.
        """
        target = player if player is not None else self.winner
        if target not in (PLAYER1, PLAYER2):
            return None
        for r in range(self.rows):
            for c in range(self.columns):
                if self.board[r][c] != target:
                    continue
                for dr, dc in ((0, 1), (1, 0), (1, 1), (1, -1)):
                    line = [(r, c)]
                    rr, cc = r + dr, c + dc
                    while (0 <= rr < self.rows and 0 <= cc < self.columns
                           and self.board[rr][cc] == target):
                        line.append((rr, cc))
                        rr += dr
                        cc += dc
                    if len(line) >= WIN_LENGTH:
                        return line
        return None

    # ------------------------------------------------------------------ serialization
    def to_state(self) -> dict:
        """Serialize the board and status for the API / frontend."""
        return {
            "rows": self.rows,
            "columns": self.columns,
            "board": [list(row) for row in self.board],
            "current": self.current,
            "winner": self.winner,
            "move_count": self.move_count,
            "full": self.is_full(),
        }
