from django.urls import path

from . import views

urlpatterns = [
    path("customizations/", views.customizations, name="customizations"),
    path("powerups/", views.powerups, name="powerups"),
    path("sessions/", views.create_session, name="create_session"),
    path("sessions/<int:session_id>/join/", views.join_session, name="join_session"),
    path("sessions/<int:session_id>/move/", views.session_move, name="session_move"),
    path("sessions/<int:session_id>/finish/", views.finish_session, name="finish_session"),
    path("ai/decision/", views.ai_decision, name="ai_decision"),
    path("connect4/matchmaking/", views.connect4_matchmaking, name="connect4_matchmaking"),
]
