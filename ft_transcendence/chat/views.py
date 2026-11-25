"""
Chat API: channels (public/private/password-protected), membership, messages.
"""
import hashlib
import hmac

from django.contrib.auth.hashers import check_password
from rest_framework import status
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import IsAuthenticated
from rest_framework.response import Response

from .models import Channel, ChannelMember, ChannelMessage
from accounts.views import _sanitize_xss


def _channel_dict(c):
    return {
        "id": c.id, "name": c.name, "type": c.type,
        "members": c.members.count(),
    }


@api_view(["GET", "POST"])
@permission_classes([IsAuthenticated])
def channels(request):
    if request.method == "GET":
        mine = ChannelMember.objects.filter(user=request.user).select_related("channel")
        return Response({
            "channels": [_channel_dict(m.channel) for m in mine],
            "public": [_channel_dict(c) for c in Channel.objects.filter(type=Channel.PUBLIC)],
        })
    data = request.data
    name = (data.get("name") or "").strip()
    ctype = data.get("type", Channel.PUBLIC)
    if not name or len(name) > 64:
        return Response({"error": "invalid channel name"},
                        status=status.HTTP_400_BAD_REQUEST)
    if ctype not in (Channel.PUBLIC, Channel.PRIVATE, Channel.PASSWORD):
        return Response({"error": "invalid channel type"},
                        status=status.HTTP_400_BAD_REQUEST)
    c = Channel.objects.create(
        name=name, type=ctype,
        password=hashlib.sha256((data.get("password") or "").encode()).hexdigest()
        if ctype == Channel.PASSWORD else "",
    )
    ChannelMember.objects.create(channel=c, user=request.user)
    return Response({"id": c.id, "name": c.name, "type": c.type},
                    status=status.HTTP_201_CREATED)


@api_view(["POST"])
@permission_classes([IsAuthenticated])
def join_channel(request, channel_id):
    """Join a channel, verifying the password if required."""
    try:
        c = Channel.objects.get(id=channel_id)
    except Channel.DoesNotExist:
        return Response({"error": "not found"}, status=status.HTTP_404_NOT_FOUND)
    if c.type == Channel.PASSWORD:
        provided = request.data.get("password") or ""
        provided_hash = hashlib.sha256(provided.encode()).hexdigest()
        if not hmac.compare_digest(provided_hash, c.password):
            return Response({"error": "invalid password"},
                            status=status.HTTP_403_FORBIDDEN)
    if ChannelMember.objects.filter(channel=c, user=request.user).exists():
        return Response({"joined": True})
    ChannelMember.objects.create(channel=c, user=request.user)
    return Response({"joined": True})


@api_view(["GET", "POST"])
@permission_classes([IsAuthenticated])
def channel_messages(request, channel_id):
    try:
        c = Channel.objects.get(id=channel_id)
    except Channel.DoesNotExist:
        return Response({"error": "not found"}, status=status.HTTP_404_NOT_FOUND)
    if not ChannelMember.objects.filter(channel=c, user=request.user).exists():
        return Response({"error": "not a member"}, status=status.HTTP_403_FORBIDDEN)
    if request.method == "GET":
        msgs = c.messages.all()[:100]
        return Response({
            "messages": [
                {"sender": m.sender.username, "content": m.content,
                 "created_at": m.created_at.isoformat()}
                for m in msgs
            ]
        })
    content = _sanitize_xss((request.data.get("content") or "").strip())
    if not content:
        return Response({"error": "empty message"}, status=status.HTTP_400_BAD_REQUEST)
    ChannelMessage.objects.create(channel=c, sender=request.user, content=content)
    return Response({"sent": True}, status=status.HTTP_201_CREATED)
