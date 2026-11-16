"""ex_project URL Configuration"""

from django.contrib import admin
from django.urls import path
from tips_app import views

urlpatterns = [
    path('admin/', admin.site.urls),
    path('', views.home, name='home'),
    path('register/', views.register, name='register'),
    path('login/', views.login_view, name='login'),
    path('logout/', views.logout_view, name='logout'),
    path('vote/<int:tip_id>/<str:action>/', views.vote, name='vote'),
    path('delete/<int:tip_id>/', views.delete_tip, name='delete_tip'),
]
