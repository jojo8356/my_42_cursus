"""hello_project URL Configuration"""

from django.contrib import admin
from django.urls import path
from helloworld.views import hello_world

urlpatterns = [
    path('admin/', admin.site.urls),
    path('helloworld/', hello_world, name='hello_world'),
]
