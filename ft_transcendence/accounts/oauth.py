"""
OAuth 2.0 authentication with 42 intra.

Implements the authorization-code flow:
  1. Build the authorization URL the user is redirected to.
  2. Exchange the authorization code for an access token.
  3. Fetch the user profile and link/create the local account.

The 42 intra endpoints are configurable via settings/env. Because the real
42 intra network is not reachable outside the school, a ``MockOAuthProvider``
is provided so the whole flow is testable end-to-end without external access.
"""
import json
import urllib.parse
import urllib.request

from django.conf import settings


def build_authorization_url(state):
    """Return the 42 OAuth authorization URL with the given state."""
    params = {
        "client_id": settings.OAUTH_42_CLIENT_ID,
        "redirect_uri": settings.OAUTH_42_REDIRECT_URI,
        "response_type": "code",
        "scope": "public",
        "state": state,
    }
    return f"{settings.OAUTH_42_AUTH_URL}?{urllib.parse.urlencode(params)}"


class OAuthError(Exception):
    pass


def exchange_code_for_token(code):
    """Exchange an authorization code for an access token at 42 intra."""
    data = urllib.parse.urlencode({
        "grant_type": "authorization_code",
        "code": code,
        "redirect_uri": settings.OAUTH_42_REDIRECT_URI,
        "client_id": settings.OAUTH_42_CLIENT_ID,
        "client_secret": settings.OAUTH_42_CLIENT_SECRET,
    }).encode()
    req = urllib.request.Request(
        settings.OAUTH_42_TOKEN_URL, data=data,
        headers={"Content-Type": "application/x-www-form-urlencoded"},
    )
    with urllib.request.urlopen(req) as resp:
        payload = json.loads(resp.read().decode())
    if "access_token" not in payload:
        raise OAuthError("Token exchange failed: no access_token")
    return payload


def fetch_user_profile(access_token):
    """Fetch the authenticated 42 intra user profile."""
    req = urllib.request.Request(
        settings.OAUTH_42_USER_URL,
        headers={"Authorization": f"Bearer {access_token}"},
    )
    with urllib.request.urlopen(req) as resp:
        return json.loads(resp.read().decode())


def oauth_login(code):
    """Full OAuth login: exchange code, fetch profile, return normalized data.

    Returns a dict with keys: login, display_name, access_token.
    """
    token_payload = exchange_code_for_token(code)
    access_token = token_payload["access_token"]
    profile = fetch_user_profile(access_token)
    return {
        "login": profile.get("login"),
        "display_name": profile.get("display_name") or profile.get("login"),
        "access_token": access_token,
    }


# ---------------------------------------------------------------------------
# Mock provider - used for tests and local development when 42 intra is
# unreachable. It reproduces the exact response shapes of the real endpoints.
# ---------------------------------------------------------------------------
class MockOAuthProvider:
    """Deterministic mock of the 42 OAuth endpoints for testing."""

    MOCK_USER = {
        "login": "mock42user",
        "display_name": "Mock 42 User",
        "email": "mock42user@student.42.fr",
    }

    @staticmethod
    def exchange_code(code):
        if not code or code == "bad_code":
            raise OAuthError("Invalid authorization code")
        return {"access_token": "mock-access-token-" + code, "token_type": "Bearer"}

    @staticmethod
    def profile(access_token):
        if not access_token:
            raise OAuthError("Missing access token")
        return dict(MockOAuthProvider.MOCK_USER)


def mock_oauth_login(code):
    """Run the OAuth flow against the mock provider (test/dev only)."""
    if code == "bad_code":
        raise OAuthError("Invalid authorization code")
    token_payload = MockOAuthProvider.exchange_code(code)
    profile = MockOAuthProvider.profile(token_payload["access_token"])
    return {
        "login": profile["login"],
        "display_name": profile["display_name"],
        "access_token": token_payload["access_token"],
    }
