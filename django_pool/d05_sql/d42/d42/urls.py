"""d42 URL Configuration"""

from django.contrib import admin
from django.urls import path, include

urlpatterns = [
    path('admin/', admin.site.urls),
    path('ex00/', include('ex00.urls')),
    path('ex01/', include('ex01.urls')),
    path('ex02/', include('ex02.urls')),
    path('ex03/', include('ex03.urls')),
    path('ex04/', include('ex04.urls')),
    path('ex05/', include('ex05.urls')),
    path('ex06/', include('ex06.urls')),
    path('ex07/', include('ex07.urls')),
    path('ex08/', include('ex08.urls')),
    path('ex09/', include('ex09.urls')),
    path('ex10/', include('ex10.urls')),
]
