"""
Games API: game sessions, customization options, AI opponent control.
"""
from rest_framework import status
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import IsAuthenticated
from rest_framework.response import Response

from .engine import PongGame
from .ai import PongAI
from .connect4 import Connect4
from .connect4_ai import Connect4AI
from .room import get_room
from .models import GameSession, GameCustomization, PowerUpTemplate


def _default_customization():
    preset, _ = GameCustomization.objects.get_or_create(
        name="default", defaults={"preset": GameCustomization.PRESET_DEFAULT}
    )
    return preset


@api_view(["GET", "POST"])
@permission_classes([IsAuthenticated])
def customizations(request):
    """List or create game customization presets (Game customization module)."""
    if request.method == "GET":
        items = GameCustomization.objects.all()
        return Response({
            "customizations": [
                {
                    "id": c.id, "name": c.name, "preset": c.preset,
                    "enable_powerups": c.enable_powerups,
                    "enable_attacks": c.enable_attacks,
                    "enable_custom_map": c.enable_custom_map,
                }
                for c in items
            ]
        })
    data = request.data
    name = data.get("name", "custom")
    preset, created = GameCustomization.objects.get_or_create(
        name=name,
        defaults={
            "enable_powerups": data.get("enable_powerups", True),
            "enable_attacks": data.get("enable_attacks", False),
            "enable_custom_map": data.get("enable_custom_map", False),
        },
    )
    return Response({"id": preset.id, "name": preset.name}, status=status.HTTP_201_CREATED)


@api_view(["GET"])
@permission_classes([IsAuthenticated])
def powerups(request):
    items = PowerUpTemplate.objects.all()
    if not items:
        for kind, label in PowerUpTemplate.KIND_CHOICES:
            PowerUpTemplate.objects.create(name=kind, kind=kind)
        items = PowerUpTemplate.objects.all()
    return Response({
        "powerups": [
            {"id": p.id, "name": p.name, "kind": p.kind, "duration": p.duration}
            for p in items
        ]
    })


@api_view(["POST"])
@permission_classes([IsAuthenticated])
def create_session(request):
    """Create a live game session (local 2-player or vs AI).

    ``kind`` selects the game: ``pong`` (default) or ``connect4`` (the
    second game required by the subject, module 7).
    """
    name = request.data.get("name") or f"game-{GameSession.objects.count() + 1}"
    is_ai = bool(request.data.get("ai", False))
    kind = request.data.get("kind", GameSession.PONG)
    if kind not in (GameSession.PONG, GameSession.CONNECT4):
        return Response({"error": "invalid game kind"},
                        status=status.HTTP_400_BAD_REQUEST)
    custom = _default_customization()
    session = GameSession.objects.create(
        name=name, kind=kind, player1=request.user, is_ai=is_ai,
        customization=custom,
    )
    # Pre-create the authoritative game room so the first websocket client
    # connects to the shared, server-owned state (Remote players module).
    room = get_room(
        session.id, kind=kind, is_ai=is_ai,
        enable_powerups=custom.enable_powerups,
        enable_custom_map=custom.enable_custom_map,
    )
    return Response({
        "id": session.id,
        "name": session.name,
        "kind": session.kind,
        "is_ai": session.is_ai,
        "state": room.state(),
    }, status=status.HTTP_201_CREATED)


@api_view(["POST"])
@permission_classes([IsAuthenticated])
def join_session(request, session_id):
    """Join a game session as player 2 (or as the second remote player)."""
    try:
        session = GameSession.objects.get(id=session_id)
    except GameSession.DoesNotExist:
        return Response({"error": "session not found"}, status=status.HTTP_404_NOT_FOUND)
    if session.player2 is not None and session.player2_id != request.user.id:
        return Response({"error": "session full"}, status=status.HTTP_409_CONFLICT)
    session.player2 = request.user
    session.status = GameSession.PLAYING
    session.save()
    return Response({"joined": True, "session_id": session.id})


@api_view(["POST"])
@permission_classes([IsAuthenticated])
def ai_decision(request):
    """Run one AI decision tick (for server-side AI or testing).

    Body: {"side": "left"/"right", "elapsed": <seconds>, ...game state}
    Returns the AI's move direction and the resulting paddle position.
    """
    side = request.data.get("side", "right")
    elapsed = float(request.data.get("elapsed", 0.0))
    game = PongGame()
    # populate from request
    if "left_paddle_y" in request.data:
        game.left_paddle.y = float(request.data["left_paddle_y"])
    if "right_paddle_y" in request.data:
        game.right_paddle.y = float(request.data["right_paddle_y"])
    if "ball_x" in request.data:
        game.ball.x = float(request.data["ball_x"])
    if "ball_y" in request.data:
        game.ball.y = float(request.data["ball_y"])
    if "ball_vx" in request.data:
        game.ball_vx = float(request.data["ball_vx"])
    if "ball_vy" in request.data:
        game.ball_vy = float(request.data["ball_vy"])
    game.state = "playing"
    ai = PongAI(side=side)
    direction = ai.act(game, elapsed)
    paddle = game.left_paddle if side == "left" else game.right_paddle
    return Response({"direction": direction, "paddle_y": paddle.y})


# --------------------------------------------------------------------------- Connect-4 (module 7)

def _session_or_404(session_id):
    try:
        return GameSession.objects.get(id=session_id)
    except GameSession.DoesNotExist:
        return None


@api_view(["POST"])
@permission_classes([IsAuthenticated])
def session_move(request, session_id):
    """Drop a Connect-4 piece in a column (turn-based second game).

    Body: ``{"column": <int>}``. Returns the updated board state and the
    resolved winner (0 = still playing, 1/2 = player, 3 = draw).
    """
    session = _session_or_404(session_id)
    if session is None:
        return Response({"error": "session not found"},
                        status=status.HTTP_404_NOT_FOUND)
    if session.kind != GameSession.CONNECT4:
        return Response({"error": "not a connect4 session"},
                        status=status.HTTP_400_BAD_REQUEST)
    if session.status == GameSession.FINISHED:
        return Response({"error": "session already finished"},
                        status=status.HTTP_400_BAD_REQUEST)
    game = Connect4()
    if "board" in request.data and request.data["board"]:
        game.board = [list(row) for row in request.data["board"]]
        game.current = int(request.data.get("current", 1))
        game.winner = int(request.data.get("winner", 0))
        game.move_count = int(request.data.get("move_count", 0))
    column = request.data.get("column")
    try:
        column = int(column)
    except (TypeError, ValueError):
        return Response({"error": "invalid column"},
                        status=status.HTTP_400_BAD_REQUEST)
    if not game.drop(column):
        return Response({"error": "invalid move"},
                        status=status.HTTP_400_BAD_REQUEST)
    # Record the move on the session (player1 is "left" = player 1).
    if game.winner in (1, 2):
        session.winner_id = (session.player1_id if game.winner == 1
                             else session.player2_id)
        session.status = GameSession.FINISHED
    elif game.winner == 3:
        session.status = GameSession.FINISHED
    session.save()
    return Response({"state": game.to_state(),
                     "session_status": session.status}, status=status.HTTP_200_OK)


@api_view(["POST"])
@permission_classes([IsAuthenticated])
def finish_session(request, session_id):
    """Finish a game and record the result in the user's match history.

    Body: ``{"winner": <user_id>}``. Updates wins/losses and writes a
    MatchHistory entry for both players (secure, up-to-date storage).
    """
    from accounts.models import User, MatchHistory
    session = _session_or_404(session_id)
    if session is None:
        return Response({"error": "session not found"},
                        status=status.HTTP_404_NOT_FOUND)
    if session.status == GameSession.FINISHED:
        return Response({"error": "session already finished"},
                        status=status.HTTP_400_BAD_REQUEST)
    winner_id = request.data.get("winner")
    p1 = session.player1
    p2 = session.player2
    if winner_id not in (p1.id if p1 else None, p2.id if p2 else None):
        return Response({"error": "invalid winner"},
                        status=status.HTTP_400_BAD_REQUEST)
    loser_id = p2.id if winner_id == p1.id else p1.id
    winner = User.objects.get(id=winner_id)
    loser = User.objects.get(id=loser_id)
    winner.wins += 1
    loser.losses += 1
    winner.save()
    loser.save()
    MatchHistory.objects.create(
        user=winner, opponent=loser, result="win",
        user_score=1, opponent_score=0,
    )
    MatchHistory.objects.create(
        user=loser, opponent=winner, result="loss",
        user_score=0, opponent_score=1,
    )
    session.status = GameSession.FINISHED
    session.winner_id = winner_id
    session.save()
    return Response({"finished": True, "winner": winner_id},
                    status=status.HTTP_200_OK)


# process-wide Connect-4 matchmaking singleton
_CONNECT4_MATCHMAKING = None


def _get_connect4_queue():
    global _CONNECT4_MATCHMAKING
    if _CONNECT4_MATCHMAKING is None:
        from tournaments.bracket import MatchmakingQueue
        _CONNECT4_MATCHMAKING = MatchmakingQueue()
    return _CONNECT4_MATCHMAKING


@api_view(["GET", "POST"])
@permission_classes([IsAuthenticated])
def connect4_matchmaking(request):
    """Matchmaking queue for balanced Connect-4 games (module 7).

    POST joins the queue; when two players are waiting a session is created
    and the match is announced. GET reports the queue and next match.
    """
    if request.method == "GET":
        queue = _get_connect4_queue()
        match = queue.next_match()
        return Response({"queue_size": queue.queue_size(),
                         "next_match": list(match) if match else None})
    queue = _get_connect4_queue()
    player_id = request.user.id
    match = queue.join(player_id)
    if not match:
        return Response({"matched": False,
                         "queue_size": queue.queue_size()})
    a_id, b_id = match
    session = GameSession.objects.create(
        name=f"connect4-{GameSession.objects.count() + 1}",
        kind=GameSession.CONNECT4,
        status=GameSession.PLAYING,
        player1_id=a_id, player2_id=b_id,
    )
    return Response({"matched": True, "match": [a_id, b_id],
                     "session_id": session.id}, status=status.HTTP_201_CREATED)
