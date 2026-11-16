from django.urls import path
from . import views

urlpatterns = [
    path('', views.index, name='ex10_index'),
]
