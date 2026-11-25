"""
ASGI config for ft_transcendence.
Routes HTTP to Django and WebSocket traffic to the Channels consumers.
"""
import os

from django.core.asgi import get_asgi_application
from channels.routing import ProtocolTypeRouter, URLRouter
from channels.auth import AuthMiddlewareStack

os.environ.setdefault("DJANGO_SETTINGS_MODULE", "config.settings")

django_asgi_app = get_asgi_application()

# Import consumers after the ASGI app is ready (avoids app-loading issues)
from games.consumers import GameConsumer          # noqa: E402
from chat.consumers import ChatConsumer            # noqa: E402
from games.routing import websocket_urlpatterns as game_ws  # noqa: E402
from chat.routing import websocket_urlpatterns as chat_ws    # noqa: E402

application = ProtocolTypeRouter({
    "http": django_asgi_app,
    "websocket": AuthMiddlewareStack(
        URLRouter(game_ws + chat_ws)
    ),
})
