"""
Accounts API: registration, login, JWT refresh, OAuth 2.0 with 42,
profile, friends, avatars, match history, GDPR, two-factor auth.
"""
import re
import secrets

from django.contrib.auth import authenticate
from django.db import IntegrityError
from rest_framework import status
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import AllowAny, IsAuthenticated
from rest_framework.response import Response

from .authentication import (
    create_token_pair, create_access_token, verify_refresh_token,
    decode_token,
)
from .models import (
    User, Friend, Block, Message, MatchHistory, ProfileVisit,
)
from .oauth import build_authorization_url, oauth_login, OAuthError


# --------------------------------------------------------------------------- helpers
USERNAME_RE = re.compile(r"^[A-Za-z0-9_.-]{3,32}$")


def _validate_registration(data):
    errors = {}
    email = (data.get("email") or "").strip().lower()
    username = (data.get("username") or "").strip()
    password = data.get("password") or ""
    if not email or "@" not in email:
        errors["email"] = "A valid email is required."
    if not username or not USERNAME_RE.match(username):
        errors["username"] = (
            "3-32 chars, letters, digits, _ . - only."
        )
    if len(password) < 8:
        errors["password"] = "Password must be at least 8 characters."
    else:
        # enforce Django's strong-password validators (numeric, common, etc.)
        from django.contrib.auth.password_validation import validate_password
        try:
            validate_password(password)
        except Exception as exc:
            errors["password"] = list(exc.messages)[0]
    return errors


def _user_public(user):
    return {
        "id": user.id,
        "email": user.email,
        "username": user.username,
        "avatar": user.avatar.url if user.avatar else "",
        "wins": user.wins,
        "losses": user.losses,
        "language": user.language,
        "is_anonymized": user.is_anonymized,
        "two_factor_enabled": user.two_factor_enabled,
    }


def _user_public_profile(user):
    """Public-facing profile: email is hidden for other users' profiles."""
    data = _user_public(user)
    data.pop("email", None)
    return data


# --------------------------------------------------------------------------- auth
@api_view(["POST"])
@permission_classes([AllowAny])
def register(request):
    """Create a new account (Standard user management module)."""
    errors = _validate_registration(request.data)
    if errors:
        return Response(errors, status=status.HTTP_400_BAD_REQUEST)
    email = request.data["email"].strip().lower()
    username = request.data["username"].strip()
    try:
        user = User.objects.create_user(
            email=email, username=username, password=request.data["password"]
        )
    except IntegrityError:
        return Response(
            {"error": "Email or username already taken"},
            status=status.HTTP_409_CONFLICT,
        )
    return Response(
        {"user": _user_public(user), "tokens": create_token_pair(user)},
        status=status.HTTP_201_CREATED,
    )


@api_view(["POST"])
@permission_classes([AllowAny])
def login(request):
    """Authenticate with email + password, return a JWT token pair."""
    email = (request.data.get("email") or "").strip().lower()
    password = request.data.get("password") or ""
    user = authenticate(request, email=email, password=password)
    if user is None or not user.is_active:
        return Response(
            {"error": "Invalid credentials"}, status=status.HTTP_401_UNAUTHORIZED
        )
    return Response({"tokens": create_token_pair(user)})


@api_view(["POST"])
@permission_classes([AllowAny])
def refresh(request):
    """Exchange a refresh token for a new access token."""
    refresh_token = request.data.get("refresh")
    if not refresh_token:
        return Response({"error": "refresh token required"},
                        status=status.HTTP_400_BAD_REQUEST)
    try:
        payload = verify_refresh_token(refresh_token)
    except Exception:
        return Response({"error": "Invalid refresh token"},
                        status=status.HTTP_401_UNAUTHORIZED)
    # check the token has not been revoked (logout)
    from accounts.models import TokenBlacklist
    import hashlib
    token_hash = hashlib.sha256(refresh_token.encode()).hexdigest()
    if TokenBlacklist.objects.filter(token_hash=token_hash).exists():
        return Response({"error": "Token has been revoked"},
                        status=status.HTTP_401_UNAUTHORIZED)
    try:
        user = User.objects.get(id=payload["sub"], is_active=True)
    except User.DoesNotExist:
        return Response({"error": "User not found"}, status=status.HTTP_401_UNAUTHORIZED)
    return Response({"access": create_access_token(user)})


@api_view(["POST"])
@permission_classes([AllowAny])
def logout(request):
    """Revoke a refresh token (logout)."""
    refresh_token = request.data.get("refresh")
    if not refresh_token:
        return Response({"error": "refresh token required"},
                        status=status.HTTP_400_BAD_REQUEST)
    try:
        payload = verify_refresh_token(refresh_token)
    except Exception:
        # logging out with an invalid/expired token is a no-op (still 200)
        return Response({"revoked": True})
    from accounts.models import TokenBlacklist
    import hashlib
    token_hash = hashlib.sha256(refresh_token.encode()).hexdigest()
    TokenBlacklist.objects.get_or_create(
        token_hash=token_hash, defaults={"user_id": payload["sub"]}
    )
    return Response({"revoked": True})


# --------------------------------------------------------------------------- OAuth
@api_view(["GET"])
@permission_classes([AllowAny])
def oauth_authorize(request):
    """Begin the 42 OAuth flow: redirect the user to the 42 authorization page."""
    state = secrets.token_urlsafe(16)
    return Response({"authorization_url": build_authorization_url(state), "state": state})


@api_view(["POST"])
@permission_classes([AllowAny])
def oauth_callback(request):
    """Handle the OAuth callback: exchange code, create/link account, login."""
    code = request.data.get("code")
    if not code:
        return Response({"error": "code required"}, status=status.HTTP_400_BAD_REQUEST)
    try:
        profile = oauth_login(code)
    except OAuthError as exc:
        return Response({"error": str(exc)}, status=status.HTTP_400_BAD_REQUEST)
    login_id = profile["login"]
    user, created = User.objects.get_or_create(
        oauth_id=login_id,
        defaults={
            "email": f"{login_id}@student.42.fr",
            "username": profile["display_name"] or login_id,
        },
    )
    if created:
        user.set_unusable_password()
        user.oauth_token = profile["access_token"]
        user.save()
    return Response({"user": _user_public(user), "tokens": create_token_pair(user)})


# --------------------------------------------------------------------------- profile
@api_view(["GET", "PUT"])
@permission_classes([IsAuthenticated])
def profile(request):
    """View or update the current user's profile."""
    if request.method == "GET":
        return Response({"user": _user_public(request.user)})
    data = request.data
    if "username" in data:
        username = data["username"].strip()
        if not USERNAME_RE.match(username):
            return Response({"username": "Invalid username"},
                            status=status.HTTP_400_BAD_REQUEST)
        if User.objects.filter(username=username).exclude(id=request.user.id).exists():
            return Response({"username": "Username taken"},
                            status=status.HTTP_409_CONFLICT)
        request.user.username = username
    if "language" in data:
        request.user.language = data["language"]
    request.user.save()
    return Response({"user": _user_public(request.user)})


@api_view(["GET"])
@permission_classes([IsAuthenticated])
def user_profile(request, user_id):
    """View another user's public profile (accessed via chat/friends)."""
    try:
        target = User.objects.get(id=user_id, is_active=True)
    except User.DoesNotExist:
        return Response({"error": "not found"}, status=status.HTTP_404_NOT_FOUND)
    ProfileVisit.objects.get_or_create(visitor=request.user, visited=target)
    return Response({"user": _user_public_profile(target)})


# --------------------------------------------------------------------------- friends
@api_view(["GET", "POST"])
@permission_classes([IsAuthenticated])
def friends(request):
    if request.method == "GET":
        accepted = Friend.objects.filter(
            user=request.user, status=Friend.ACCEPTED
        ).select_related("friend")
        pending_in = Friend.objects.filter(
            friend=request.user, status=Friend.PENDING
        ).select_related("user")
        return Response({
            "friends": [_user_public(f.friend) for f in accepted],
            "pending": [_user_public(f.user) for f in pending_in],
        })
    # POST: send or accept a friend request
    target_id = request.data.get("user_id")
    try:
        target = User.objects.get(id=target_id, is_active=True)
    except (User.DoesNotExist, ValueError):
        return Response({"error": "user not found"}, status=status.HTTP_404_NOT_FOUND)
    if target.id == request.user.id:
        return Response({"error": "cannot friend yourself"},
                        status=status.HTTP_400_BAD_REQUEST)

    # Determine the action: accept / reject an incoming request, or send.
    action = request.data.get("action") or ("accept" if request.data.get("accept") else None)

    # Look for an incoming pending request from the target to us.
    incoming = Friend.objects.filter(
        user=target, friend=request.user, status=Friend.PENDING
    ).first()

    if action in ("accept", "reject"):
        if not incoming:
            return Response({"error": "no pending request from this user"},
                            status=status.HTTP_400_BAD_REQUEST)
        if action == "accept":
            incoming.status = Friend.ACCEPTED
        else:
            incoming.status = Friend.REJECTED
        incoming.save()
        return Response({"status": incoming.status})

    # Send a new request.
    # Cannot send if we already have a pending request to this user.
    existing_out = Friend.objects.filter(
        user=request.user, friend=target, status=Friend.PENDING
    ).first()
    if existing_out:
        return Response({"error": "request already pending"},
                        status=status.HTTP_400_BAD_REQUEST)
    # Cannot send if already accepted friends.
    accepted = Friend.objects.filter(
        user=request.user, friend=target, status=Friend.ACCEPTED
    ).first()
    if accepted:
        return Response({"error": "already friends"}, status=status.HTTP_400_BAD_REQUEST)
    rel, created = Friend.objects.get_or_create(
        user=request.user, friend=target, defaults={"status": Friend.PENDING}
    )
    # If a previously rejected request exists, reset it to pending (resend).
    if not created and rel.status == Friend.REJECTED:
        rel.status = Friend.PENDING
        rel.save()
        created = True
    if not created:
        return Response({"error": "request already exists"},
                        status=status.HTTP_400_BAD_REQUEST)
    return Response({"status": rel.status}, status=status.HTTP_201_CREATED)


# --------------------------------------------------------------------------- blocking
@api_view(["POST", "DELETE"])
@permission_classes([IsAuthenticated])
def blocks(request):
    """Block / unblock a user (chat blocking module)."""
    target_id = request.data.get("user_id")
    try:
        target = User.objects.get(id=target_id)
    except (User.DoesNotExist, ValueError):
        return Response({"error": "user not found"}, status=status.HTTP_404_NOT_FOUND)
    if request.method == "POST":
        Block.objects.get_or_create(user=request.user, blocked_user=target)
        return Response({"blocked": True})
    Block.objects.filter(user=request.user, blocked_user=target).delete()
    return Response({"blocked": False})


# --------------------------------------------------------------------------- messages
@api_view(["GET", "POST"])
@permission_classes([IsAuthenticated])
def messages(request, user_id):
    """Direct messages with a specific user."""
    try:
        peer = User.objects.get(id=user_id)
    except (User.DoesNotExist, ValueError):
        return Response({"error": "user not found"}, status=status.HTTP_404_NOT_FOUND)
    # respect blocking
    if Block.objects.filter(user=peer, blocked_user=request.user).exists():
        return Response({"error": "blocked by peer"}, status=status.HTTP_403_FORBIDDEN)
    if request.method == "GET":
        from django.db.models import Q
        msgs = Message.objects.filter(
            Q(sender=request.user, recipient=peer)
            | Q(sender=peer, recipient=request.user)
        ).order_by("created_at")
        return Response({
            "messages": [
                {"sender": m.sender_id, "content": m.content,
                 "created_at": m.created_at.isoformat()}
                for m in msgs
            ]
        })
    content = (request.data.get("content") or "").strip()
    if not content:
        return Response({"error": "empty message"}, status=status.HTTP_400_BAD_REQUEST)
    # XSS protection: strip dangerous markup server-side
    content = _sanitize_xss(content)
    Message.objects.create(sender=request.user, recipient=peer, content=content)
    return Response({"sent": True}, status=status.HTTP_201_CREATED)


def _sanitize_xss(text):
    """Basic server-side XSS sanitization: strip dangerous markup."""
    import html
    return html.escape(text)


# --------------------------------------------------------------------------- match history
@api_view(["GET"])
@permission_classes([IsAuthenticated])
def match_history(request):
    matches = MatchHistory.objects.filter(user=request.user).select_related("opponent")
    return Response({
        "matches": [
            {
                "opponent": m.opponent.username,
                "result": m.result,
                "user_score": m.user_score,
                "opponent_score": m.opponent_score,
                "played_at": m.played_at.isoformat(),
            }
            for m in matches
        ],
        "wins": request.user.wins,
        "losses": request.user.losses,
    })


# --------------------------------------------------------------------------- 2FA
@api_view(["POST"])
@permission_classes([IsAuthenticated])
def two_factor_enable(request):
    """Enable two-factor authentication (generates a TOTP secret)."""
    import base64
    secret = base64.b32encode(secrets.token_bytes(10)).decode()
    request.user.totp_secret = secret
    request.user.two_factor_enabled = True
    request.user.save()
    return Response({"secret": secret, "enabled": True})


@api_view(["POST"])
@permission_classes([IsAuthenticated])
def two_factor_disable(request):
    request.user.two_factor_enabled = False
    request.user.totp_secret = None
    request.user.save()
    return Response({"enabled": False})


# --------------------------------------------------------------------------- GDPR
@api_view(["POST"])
@permission_classes([IsAuthenticated])
def anonymize(request):
    """GDPR: anonymize the user's personal data."""
    request.user.anonymize()
    return Response({"anonymized": True})


@api_view(["POST"])
@permission_classes([IsAuthenticated])
def delete_account(request):
    """GDPR: permanently delete the account and all associated data."""
    request.user.delete_account()
    return Response({"deleted": True})
