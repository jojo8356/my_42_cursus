"""
JWT authentication for ft_transcendence.
Access + refresh token pair, signed with HS256.
"""
import datetime

import jwt
from django.conf import settings
from rest_framework import authentication, exceptions


def _make_token(user, ttl, token_type):
    now = datetime.datetime.now(datetime.timezone.utc)
    payload = {
        "sub": str(user.id),
        "email": user.email,
        "username": user.username,
        "type": token_type,
        "iat": int(now.timestamp()),
        "exp": int((now + datetime.timedelta(seconds=ttl)).timestamp()),
    }
    return jwt.encode(payload, settings.JWT_SECRET, algorithm=settings.JWT_ALGORITHM)


def create_access_token(user):
    return _make_token(user, settings.JWT_ACCESS_TTL, "access")


def create_refresh_token(user):
    return _make_token(user, settings.JWT_REFRESH_TTL, "refresh")


def create_token_pair(user):
    return {
        "access": create_access_token(user),
        "refresh": create_refresh_token(user),
    }


def decode_token(token):
    return jwt.decode(token, settings.JWT_SECRET, algorithms=[settings.JWT_ALGORITHM])


def verify_refresh_token(token):
    payload = decode_token(token)
    if payload.get("type") != "refresh":
        raise exceptions.AuthenticationFailed("Invalid token type")
    return payload


class JWTAuthentication(authentication.BaseAuthentication):
    """Authenticate via the Authorization: Bearer <access_token> header."""

    keyword = "Bearer"

    def authenticate(self, request):
        header = authentication.get_authorization_header(request).split()
        if not header:
            return None
        if header[0].lower() != self.keyword.lower().encode():
            return None
        if len(header) == 1:
            raise exceptions.AuthenticationFailed("Invalid authorization header")
        if len(header) > 2:
            raise exceptions.AuthenticationFailed("Invalid authorization header")
        try:
            token = header[1].decode()
        except UnicodeError:
            raise exceptions.AuthenticationFailed("Invalid token")
        try:
            payload = decode_token(token)
        except Exception:
            raise exceptions.AuthenticationFailed("Invalid or expired token")
        if payload.get("type") != "access":
            return None
        from accounts.models import User

        try:
            user = User.objects.get(id=payload["sub"], is_active=True)
        except User.DoesNotExist:
            raise exceptions.AuthenticationFailed("User not found")
        return (user, token)

    def authenticate_header(self, request):
        return self.keyword
