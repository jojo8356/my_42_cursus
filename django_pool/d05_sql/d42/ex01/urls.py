from django.urls import path
from . import views

urlpatterns = [
    path('populate', views.populate, name='ex01_populate'),
    path('display', views.display, name='ex01_display'),
]
