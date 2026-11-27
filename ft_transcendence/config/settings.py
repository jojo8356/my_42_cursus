"""
Django settings for ft_transcendence.
Full-stack Pong contest platform (42 final project, subject v3.0.1).
"""
from pathlib import Path
import os

BASE_DIR = Path(__file__).resolve().parent.parent

# SECURITY WARNING: keep the secret key used in production secret!
SECRET_KEY = os.environ.get(
    "DJANGO_SECRET_KEY",
    "django-insecure-ft-transcendence-dev-key-change-in-production",
)

# SECURITY WARNING: don't run with debug turned on in production!
DEBUG = os.environ.get("DJANGO_DEBUG", "1") == "1"

ALLOWED_HOSTS = os.environ.get("DJANGO_ALLOWED_HOSTS", "*").split(",")

# Application definition
INSTALLED_APPS = [
    "django.contrib.admin",
    "django.contrib.auth",
    "django.contrib.contenttypes",
    "django.contrib.sessions",
    "django.contrib.messages",
    "django.contrib.staticfiles",
    # third party
    "channels",
    "rest_framework",
    # local
    "accounts",
    "chat",
    "games",
    "tournaments",
]

MIDDLEWARE = [
    "django.middleware.security.SecurityMiddleware",
    "whitenoise.middleware.WhiteNoiseMiddleware",
    "django.contrib.sessions.middleware.SessionMiddleware",
    "django.middleware.common.CommonMiddleware",
    "django.middleware.csrf.CsrfViewMiddleware",
    "django.contrib.auth.middleware.AuthenticationMiddleware",
    "django.contrib.messages.middleware.MessageMiddleware",
    "django.middleware.clickjacking.XFrameOptionsMiddleware",
]

ROOT_URLCONF = "config.urls"

TEMPLATES = [
    {
        "BACKEND": "django.template.backends.django.DjangoTemplates",
        "DIRS": [BASE_DIR / "frontend"],
        "APP_DIRS": True,
        "OPTIONS": {
            "context_processors": [
                "django.template.context_processors.debug",
                "django.template.context_processors.request",
                "django.contrib.auth.context_processors.auth",
                "django.contrib.messages.context_processors.messages",
            ],
        },
    },
]

WSGI_APPLICATION = "config.wsgi.application"
ASGI_APPLICATION = "config.asgi.application"

# Database
# The subject designates PostgreSQL as the production database. For a
# self-contained, testable build we default to SQLite; PostgreSQL is selected
# automatically when the DATABASE_URL / POSTGRES env vars are present.
if os.environ.get("POSTGRES_HOST"):
    DATABASES = {
        "default": {
            "ENGINE": "django.db.backends.postgresql",
            "NAME": os.environ.get("POSTGRES_DB", "transcendence"),
            "USER": os.environ.get("POSTGRES_USER", ""),
            "PASSWORD": os.environ.get("POSTGRES_PASSWORD", ""),
            "HOST": os.environ.get("POSTGRES_HOST"),
            "PORT": os.environ.get("POSTGRES_PORT", "5432"),
        }
    }
else:
    DATABASES = {
        "default": {
            "ENGINE": "django.db.backends.sqlite3",
            "NAME": BASE_DIR / "db.sqlite3",
        }
    }

# Channels channel layer. A Redis backend is used in production (when
# REDIS_URL is set) so websockets scale across multiple ASGI workers; the
# in-memory layer keeps the app self-contained and testable without Redis.
if os.environ.get("REDIS_URL"):
    CHANNEL_LAYERS = {
        "default": {
            "BACKEND": "channels_redis.core.RedisChannelLayer",
            "CONFIG": {
                "hosts": [os.environ["REDIS_URL"]],
            },
        }
    }
else:
    CHANNEL_LAYERS = {
        "default": {
            "BACKEND": "channels.layers.InMemoryChannelLayer",
        }
    }

AUTH_USER_MODEL = "accounts.User"

AUTH_PASSWORD_VALIDATORS = [
    {"NAME": "django.contrib.auth.password_validation.UserAttributeSimilarityValidator"},
    {"NAME": "django.contrib.auth.password_validation.MinimumLengthValidator",
     "OPTIONS": {"min_length": 8}},
    {"NAME": "django.contrib.auth.password_validation.CommonPasswordValidator"},
    {"NAME": "django.contrib.auth.password_validation.NumericPasswordValidator"},
]

LANGUAGE_CODE = "en-us"
TIME_ZONE = "UTC"
USE_I18N = True
USE_TZ = True

STATIC_URL = "/static/"
STATICFILES_DIRS = [BASE_DIR / "frontend" / "static"]
STATIC_ROOT = BASE_DIR / "staticfiles"

MEDIA_URL = "/media/"
MEDIA_ROOT = BASE_DIR / "media"

DEFAULT_AUTO_FIELD = "django.db.models.BigAutoField"

# REST framework
REST_FRAMEWORK = {
    "DEFAULT_AUTHENTICATION_CLASSES": [
        "accounts.authentication.JWTAuthentication",
    ],
    "DEFAULT_PERMISSION_CLASSES": [
        "rest_framework.permissions.IsAuthenticated",
    ],
}

# JWT
JWT_SECRET = os.environ.get("JWT_SECRET", SECRET_KEY)
JWT_ALGORITHM = "HS256"
JWT_ACCESS_TTL = 3600  # 1 hour
JWT_REFRESH_TTL = 86400 * 7  # 7 days

# OAuth 2.0 with 42 (configurable; a dev/mock provider is used in tests)
OAUTH_42_CLIENT_ID = os.environ.get("OAUTH_42_CLIENT_ID", "")
OAUTH_42_CLIENT_SECRET = os.environ.get("OAUTH_42_CLIENT_SECRET", "")
OAUTH_42_AUTH_URL = os.environ.get(
    "OAUTH_42_AUTH_URL", "https://profile.intra.42.fr/oauth/authorize"
)
OAUTH_42_TOKEN_URL = os.environ.get(
    "OAUTH_42_TOKEN_URL", "https://profile.intra.42.fr/oauth/token"
)
OAUTH_42_USER_URL = os.environ.get(
    "OAUTH_42_USER_URL", "https://profile.intra.42.fr/api/v2/me"
)
OAUTH_42_REDIRECT_URI = os.environ.get(
    "OAUTH_42_REDIRECT_URI", "http://localhost:8000/api/auth/oauth/callback"
)

# --------------------------------------------------------------------------- HTTPS / WSS
# The subject makes HTTPS mandatory whenever a backend is used ("utiliser wss
# au lieu de ws"). Secure transport is enabled by the Docker container via
# DJANGO_SECURE=1. It stays off for the local dev server and the test suite
# (which run over plain HTTP) so development and tests keep working.
SECURE_TRANSPORT = os.environ.get("DJANGO_SECURE", "0") == "1"

if SECURE_TRANSPORT:
    # Force HTTPS for every request and harden cookies / HSTS.
    SECURE_SSL_REDIRECT = True
    SECURE_PROXY_SSL_HEADER = ("HTTP_X_FORWARDED_PROTO", "https")
    SESSION_COOKIE_SECURE = True
    CSRF_COOKIE_SECURE = True
    SECURE_HSTS_SECONDS = 31536000          # 1 year
    SECURE_HSTS_INCLUDE_SUBDOMAINS = True
    SECURE_HSTS_PRELOAD = True
    SECURE_CONTENT_TYPE_NOSNIFF = True
    SECURE_REFERRER_POLICY = "same-origin"
    X_FRAME_OPTIONS = "DENY"
else:
    # Defence-in-depth headers that are safe in every mode.
    SECURE_CONTENT_TYPE_NOSNIFF = True
    SECURE_REFERRER_POLICY = "same-origin"
    X_FRAME_OPTIONS = "DENY"

# WebSocket scheme: wss over TLS in production, ws for local development.
# (The ASGI server / reverse proxy terminates TLS; the frontend also picks
# wss automatically from location.protocol.)
WS_SCHEME = "wss" if SECURE_TRANSPORT else "ws"

# WhiteNoise serves the collected static files in production (DEBUG=0),
# keeping the container self-contained without a separate static server.
STORAGES = {
    "default": {"BACKEND": "django.core.files.storage.FileSystemStorage"},
    "staticfiles": {"BACKEND": "whitenoise.storage.CompressedManifestStaticFilesStorage"},
}
