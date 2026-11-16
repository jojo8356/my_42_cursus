from django.urls import path
from . import views

urlpatterns = [
    path('django/', views.django_page, name='django_page'),
    path('affichage/', views.affichage_page, name='affichage_page'),
    path('templates/', views.templates_page, name='templates_page'),
]
