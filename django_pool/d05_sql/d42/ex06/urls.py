from django.urls import path
from . import views

urlpatterns = [
    path('init', views.init, name='ex06_init'),
    path('populate', views.populate, name='ex06_populate'),
    path('display', views.display, name='ex06_display'),
    path('update', views.update, name='ex06_update'),
]
