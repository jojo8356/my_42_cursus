from django.urls import path

from . import views

urlpatterns = [
    path("", views.tournaments, name="tournaments"),
    path("<int:tournament_id>/", views.tournament_detail, name="tournament_detail"),
    path("<int:tournament_id>/start/", views.start_tournament, name="start_tournament"),
    path("<int:tournament_id>/match/", views.record_match, name="record_match"),
    path("matchmaking/", views.matchmaking, name="matchmaking"),
]
