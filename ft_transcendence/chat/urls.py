from django.urls import path

from . import views

urlpatterns = [
    path("channels/", views.channels, name="channels"),
    path("channels/<int:channel_id>/join/", views.join_channel, name="join_channel"),
    path("channels/<int:channel_id>/messages/", views.channel_messages, name="channel_messages"),
]
