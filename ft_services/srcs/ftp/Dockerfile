# ============================================================================ #
#  ft_services — FTPS container (42 School)                                  #
#  Base: Alpine Linux. Secure FTP server (FTP over SSL).                    #
# ============================================================================ #
FROM alpine:3.20

# proftpd (FTP server) + openssl (TLS for FTPS) + ca-certificates
RUN apk add --no-cache proftpd openssl ca-certificates

COPY proftpd.conf /etc/proftpd.conf
COPY entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh \
    && mkdir -p /etc/proftpd/ssl

EXPOSE 21

ENTRYPOINT ["/entrypoint.sh"]
