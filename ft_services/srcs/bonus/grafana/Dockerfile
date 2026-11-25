# ============================================================================ #
#  ft_services — Grafana container (BONUS: monitoring)                       #
#  Base: Alpine Linux. Grafana binary downloaded at build time.              #
# ============================================================================ #
FROM alpine:3.20

RUN apk add --no-cache ca-certificates tzdata

# Grafana (latest stable) downloaded from the official source — no prebuilt
# hub image of the whole service.
ARG GRAFANA_VERSION=11.2.0
RUN wget -q "https://dl.grafana.com/oss/release/grafana-${GRAFANA_VERSION}.linux-amd64.tar.gz" \
        -O /tmp/grafana.tar.gz \
    && tar -xzf /tmp/grafana.tar.gz -C /opt \
    && mv /opt/grafana-${GRAFANA_VERSION} /opt/grafana \
    && rm -f /tmp/grafana.tar.gz \
    && adduser -D -h /opt/grafana grafana \
    && chown -R grafana:grafana /opt/grafana

USER grafana
WORKDIR /opt/grafana

EXPOSE 3000

ENTRYPOINT ["/opt/grafana/bin/grafana", "server", "--homepath=/opt/grafana"]
