"""Root URL configuration for ft_transcendence."""
from django.conf import settings
from django.conf.urls.static import static
from django.contrib import admin
from django.http import HttpResponse
from django.urls import include, path
from django.views.generic import TemplateView


def spa(request, *args, **kwargs):
    """Serve the single-page application (SPA) index for the frontend."""
    # API paths that didn't match a route must 404, not fall through to the
    # SPA catch-all (keeps API behavior explicit and avoids masking errors).
    path = request.path
    if path.startswith("/api/"):
        return HttpResponse("Not found", status=404)
    index = settings.BASE_DIR / "frontend" / "index.html"
    if index.exists():
        return HttpResponse(index.read_text(encoding="utf-8"),
                            content_type="text/html")
    return HttpResponse("ft_transcendence frontend not built", status=404)


urlpatterns = [
    path("admin/", admin.site.urls),
    path("api/auth/", include("accounts.urls")),
    path("api/games/", include("games.urls")),
    path("api/tournaments/", include("tournaments.urls")),
    path("api/chat/", include("chat.urls")),
    # SPA catch-all (any non-api route renders the SPA; client-side routing
    # handles Back/Forward buttons)
    path("", spa, name="spa"),
    path("<path:path>", spa, name="spa-catchall"),
]

if settings.DEBUG:
    urlpatterns += static(settings.STATIC_URL,
                          document_root=settings.STATIC_ROOT)
    urlpatterns += static(settings.MEDIA_URL,
                          document_root=settings.MEDIA_ROOT)
