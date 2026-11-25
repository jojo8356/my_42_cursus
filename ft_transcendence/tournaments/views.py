"""
Tournaments API: create tournaments, register players, generate brackets,
run matchmaking, record match results.
"""
from rest_framework import status
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import IsAuthenticated
from rest_framework.response import Response

from .bracket import build_bracket, assign_seeding, bracket_rounds
from .models import Tournament, Participant, Match


def _tournament_dict(t):
    return {
        "id": t.id,
        "name": t.name,
        "status": t.status,
        "winner": t.winner.username if t.winner else None,
        "bracket": t.bracket,
        "participants": [
            {"id": p.id, "alias": p.alias, "seed": p.seed, "score": p.score,
             "eliminated": p.eliminated}
            for p in t.participants.all()
        ],
    }


@api_view(["GET", "POST"])
@permission_classes([IsAuthenticated])
def tournaments(request):
    if request.method == "GET":
        items = Tournament.objects.all().prefetch_related("participants")
        return Response({"tournaments": [_tournament_dict(t) for t in items]})
    name = request.data.get("name", "Tournament")
    t = Tournament.objects.create(name=name)
    return Response({"id": t.id, "name": t.name, "status": t.status},
                    status=status.HTTP_201_CREATED)


@api_view(["GET", "POST"])
@permission_classes([IsAuthenticated])
def tournament_detail(request, tournament_id):
    try:
        t = Tournament.objects.get(id=tournament_id)
    except Tournament.DoesNotExist:
        return Response({"error": "not found"}, status=status.HTTP_404_NOT_FOUND)

    if request.method == "GET":
        return Response(_tournament_dict(t))

    # POST: register the current user as a participant with an alias
    if t.status != Tournament.OPEN:
        return Response({"error": "tournament not open"},
                        status=status.HTTP_400_BAD_REQUEST)
    alias = (request.data.get("alias") or "").strip()
    if "alias" in request.data:
        # an explicitly-provided alias must be valid
        if not alias or len(alias) > 32:
            return Response({"error": "invalid alias"},
                            status=status.HTTP_400_BAD_REQUEST)
    else:
        alias = request.user.username
    if len(alias) > 32:
        return Response({"error": "invalid alias"},
                        status=status.HTTP_400_BAD_REQUEST)
    if Participant.objects.filter(tournament=t, user=request.user).exists():
        return Response({"error": "already registered"},
                        status=status.HTTP_409_CONFLICT)
    participant = Participant.objects.create(
        tournament=t, user=request.user, alias=alias,
        seed=Participant.objects.filter(tournament=t).count(),
    )
    return Response({"registered": True, "seed": participant.seed},
                    status=status.HTTP_201_CREATED)


@api_view(["POST"])
@permission_classes([IsAuthenticated])
def start_tournament(request, tournament_id):
    """Generate the bracket and start the tournament."""
    try:
        t = Tournament.objects.get(id=tournament_id)
    except Tournament.DoesNotExist:
        return Response({"error": "not found"}, status=status.HTTP_404_NOT_FOUND)
    participants = list(t.participants.all().order_by("seed"))
    if len(participants) < 2:
        return Response({"error": "need at least 2 players"},
                        status=status.HTTP_400_BAD_REQUEST)
    player_ids = [p.user_id for p in participants]
    bracket = build_bracket(len(player_ids))
    assign_seeding(bracket, player_ids)
    rounds = bracket_rounds(bracket)
    t.bracket = [
        [[m[0], m[1]] for m in round_matches] for round_matches in rounds
    ]
    t.start()
    return Response({"started": True, "bracket": t.bracket})


@api_view(["POST"])
@permission_classes([IsAuthenticated])
def record_match(request, tournament_id):
    """Record a match result and advance the bracket."""
    try:
        t = Tournament.objects.get(id=tournament_id)
    except Tournament.DoesNotExist:
        return Response({"error": "not found"}, status=status.HTTP_404_NOT_FOUND)
    player1_id = request.data.get("player1")
    player2_id = request.data.get("player2")
    winner_id = request.data.get("winner")
    score1 = int(request.data.get("score1", 0))
    score2 = int(request.data.get("score2", 0))
    if winner_id not in (player1_id, player2_id):
        return Response({"error": "invalid winner"},
                        status=status.HTTP_400_BAD_REQUEST)
    match = Match.objects.create(
        tournament=t,
        player1_id=player1_id, player2_id=player2_id,
        winner_id=winner_id, score1=score1, score2=score2,
    )
    # update stats
    from accounts.models import User, MatchHistory
    user = User.objects.get(id=winner_id)
    user.wins += 1
    user.save()
    loser_id = player2_id if winner_id == player1_id else player1_id
    MatchHistory.objects.create(
        user=User.objects.get(id=winner_id),
        opponent=User.objects.get(id=loser_id),
        result="win", user_score=score1 if winner_id == player1_id else score2,
        opponent_score=score2 if winner_id == player1_id else score1,
        tournament=t,
    )
    MatchHistory.objects.create(
        user=User.objects.get(id=loser_id),
        opponent=User.objects.get(id=winner_id),
        result="loss", user_score=score2 if winner_id == player1_id else score1,
        opponent_score=score1 if winner_id == player1_id else score2,
        tournament=t,
    )
    return Response({"recorded": True, "match_id": match.id},
                    status=status.HTTP_201_CREATED)


@api_view(["GET", "POST"])
@permission_classes([IsAuthenticated])
def matchmaking(request):
    """Matchmaking queue: join the queue, get the announced next match."""
    if request.method == "GET":
        return Response({"queue": [], "next_match": None,
                         "message": "no players waiting"})
    # In a real deployment the queue is a process-wide singleton. Here we use
    # a simple in-memory queue stored on the app module.
    queue = _get_queue()
    player_id = request.user.id
    match = queue.join(player_id)
    if match:
        return Response({"matched": True, "match": list(match)})
    return Response({"matched": False, "queue_size": queue.queue_size()})


# process-wide matchmaking singleton
_MATCHMAKING = None


def _get_queue():
    global _MATCHMAKING
    if _MATCHMAKING is None:
        from .bracket import MatchmakingQueue
        _MATCHMAKING = MatchmakingQueue()
    return _MATCHMAKING
