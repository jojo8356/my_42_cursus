from django.urls import path
from . import views

urlpatterns = [
    path('', views.index, name='ex10_index'),
    path('populate', views.populate, name='ex10_populate'),
]
