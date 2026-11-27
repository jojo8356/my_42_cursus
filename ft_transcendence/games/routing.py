from django.urls import re_path

from . import consumers
from . import connect4_consumers

websocket_urlpatterns = [
    re_path(r"ws/game/(?P<session_id>\w+)/(?P<side>left|right)/$",
            consumers.GameConsumer.as_asgi()),
    re_path(r"ws/connect4/(?P<session_id>\w+)/(?P<side>left|right)/$",
            connect4_consumers.Connect4Consumer.as_asgi()),
]
