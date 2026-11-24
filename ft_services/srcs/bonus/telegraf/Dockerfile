# ============================================================================ #
#  ft_services — Telegraf container (BONUS: metrics collector → InfluxDB)    #
#  Base: Alpine Linux. Telegraf binary downloaded at build time.             #
# ============================================================================ #
FROM alpine:3.20

RUN apk add --no-cache ca-certificates

# Telegraf — downloaded from the official source (no prebuilt hub image).
ARG TELEGRAF_VERSION=1.31.1
RUN wget -q "https://dl.influxdata.com/telegraf/releases/telegraf-${TELEGRAF_VERSION}-linux-amd64.tar.gz" \
        -O /tmp/telegraf.tar.gz \
    && tar -xzf /tmp/telegraf.tar.gz -C /opt \
    && mv /opt/telegraf-${TELEGRAF_VERSION}-linux-amd64/telegraf /usr/local/bin/telegraf \
    && rm -rf /tmp/telegraf.tar.gz /opt/telegraf-${TELEGRAF_VERSION}-linux-amd64

COPY telegraf.conf /etc/telegraf/telegraf.conf

ENTRYPOINT ["/usr/local/bin/telegraf", "--config", "/etc/telegraf/telegraf.conf"]
