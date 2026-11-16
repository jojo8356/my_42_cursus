"""d42 WSGI config."""

import os
from django.core.wsgi import get_wsgi_application

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'd42.settings')
application = get_wsgi_application()
