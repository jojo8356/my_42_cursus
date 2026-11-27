from django.urls import path

from . import views

urlpatterns = [
    path("register/", views.register, name="register"),
    path("login/", views.login, name="login"),
    path("refresh/", views.refresh, name="refresh"),
    path("logout/", views.logout, name="logout"),
    path("oauth/authorize/", views.oauth_authorize, name="oauth_authorize"),
    path("oauth/callback/", views.oauth_callback, name="oauth_callback"),
    path("profile/", views.profile, name="profile"),
    path("profile/<int:user_id>/", views.user_profile, name="user_profile"),
    path("friends/", views.friends, name="friends"),
    path("blocks/", views.blocks, name="blocks"),
    path("messages/<int:user_id>/", views.messages, name="messages"),
    path("matches/", views.match_history, name="match_history"),
    path("2fa/enable/", views.two_factor_enable, name="two_factor_enable"),
    path("2fa/disable/", views.two_factor_disable, name="two_factor_disable"),
    path("gdpr/anonymize/", views.anonymize, name="anonymize"),
    path("gdpr/delete/", views.delete_account, name="delete_account"),
    path("avatar/upload/", views.avatar_upload, name="avatar_upload"),
]
