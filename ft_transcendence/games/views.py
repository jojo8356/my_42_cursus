"""
Games API: game sessions, customization options, AI opponent control.
"""
from rest_framework import status
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import IsAuthenticated
from rest_framework.response import Response

from .engine import PongGame
from .ai import PongAI
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
    """Create a live game session (local 2-player or vs AI)."""
    name = request.data.get("name") or f"game-{GameSession.objects.count() + 1}"
    is_ai = bool(request.data.get("ai", False))
    custom = _default_customization()
    session = GameSession.objects.create(
        name=name, player1=request.user, is_ai=is_ai, customization=custom
    )
    return Response({
        "id": session.id,
        "name": session.name,
        "is_ai": session.is_ai,
        "state": PongGame(
            enable_powerups=custom.enable_powerups,
            enable_custom_map=custom.enable_custom_map,
        ).to_state(),
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
