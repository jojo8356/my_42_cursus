"""articles_project URL Configuration"""

from django.conf.urls.i18n import i18n_patterns
from django.contrib import admin
from django.urls import path

from articles_app import views

urlpatterns = [
    path('admin/', admin.site.urls),
]

# URLs i18n (préfixe /en/ ou /fr/)
urlpatterns += i18n_patterns(
    path('', views.HomeView.as_view(), name='home'),
    path('articles/', views.ArticleListView.as_view(), name='articles'),
    path('articles/<int:pk>/', views.ArticleDetailView.as_view(), name='article_detail'),
    path('publications/', views.PublicationsView.as_view(), name='publications'),
    path('favourites/', views.FavouritesView.as_view(), name='favourites'),
    path('register/', views.RegisterView.as_view(), name='register'),
    path('login/', views.LoginView.as_view(), name='login'),
    path('logout/', views.LogoutView.as_view(), name='logout'),
    path('publish/', views.PublishView.as_view(), name='publish'),
    path('articles/<int:pk>/favourite/', views.AddFavouriteView.as_view(), name='add_favourite'),
)
