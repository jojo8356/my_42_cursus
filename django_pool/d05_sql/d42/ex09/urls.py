from django.urls import path
from . import views

urlpatterns = [
    path('populate', views.populate, name='ex09_populate'),
    path('display', views.display, name='ex09_display'),
]
