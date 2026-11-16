"""d09 URL Configuration"""

from django.contrib import admin
from django.urls import path
from account import views as account_views
from chat import views as chat_views

urlpatterns = [
    path('admin/', admin.site.urls),
    path('account/', account_views.account_page, name='account'),
    path('login/', account_views.ajax_login, name='ajax_login'),
    path('logout/', account_views.ajax_logout, name='ajax_logout'),
    path('chat/', chat_views.chat_list, name='chat_list'),
    path('chat/<int:room_id>/', chat_views.chat_room, name='chat_room'),
]
