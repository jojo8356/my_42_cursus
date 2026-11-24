# ============================================================================ #
#  ft_services — NGINX container (42 School)                                 #
#  Base: Alpine Linux (lightweight). Built from scratch (no prebuilt hub).  #
#  Role: SSL web server, reverse proxy to phpMyAdmin, 307 redirect to WP.  #
# ============================================================================ #
FROM alpine:3.20

# nginx + openssl (TLS) + ca-certificates
RUN apk add --no-cache nginx openssl ca-certificates

COPY nginx.conf /etc/nginx/nginx.conf
COPY entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh \
    && mkdir -p /etc/nginx/ssl

EXPOSE 80 443

ENTRYPOINT ["/entrypoint.sh"]
