# ============================================================================ #
#  ft_services — SSH container (BONUS: secure remote access)                 #
#  Base: Alpine Linux. OpenSSH server. Port 22.                              #
# ============================================================================ #
FROM alpine:3.20

RUN apk add --no-cache openssh ca-certificates

# Generate host keys + enable password auth (credentials from Secret)
RUN ssh-keygen -A

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 22

ENTRYPOINT ["/entrypoint.sh"]
