# ft_transcendence — self-contained container (subject: one command to run).
FROM python:3.11-slim

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    DJANGO_SECURE=1 \
    DJANGO_DEBUG=0

# System deps: OpenSSL to generate the TLS cert, curl for healthcheck.
RUN apt-get update \
    && apt-get install -y --no-install-recommends openssl curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Install Python deps first (cached layer).
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy the project.
COPY . .

# Collect static files for WhiteNoise.
RUN python manage.py collectstatic --noinput

# Generate a self-signed TLS certificate at container start (entrypoint).
RUN chmod +x docker-entrypoint.sh

EXPOSE 80 443

HEALTHCHECK --interval=30s --timeout=5s --start-period=20s --retries=3 \
    CMD curl -fk https://localhost/ >/dev/null 2>&1 || exit 1

ENTRYPOINT ["./docker-entrypoint.sh"]
