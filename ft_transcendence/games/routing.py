from django.urls import re_path

from . import consumers

websocket_urlpatterns = [
    re_path(r"ws/game/(?P<session_id>\w+)/(?P<side>left|right)/$",
            consumers.GameConsumer.as_asgi()),
]
