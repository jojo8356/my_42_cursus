# ============================================================================ #
#  ft_services — InfluxDB container (BONUS: time-series DB for Grafana)      #
#  Base: Alpine Linux. InfluxDB 2.x binary downloaded at build time.         #
# ============================================================================ #
FROM alpine:3.20

RUN apk add --no-cache ca-certificates

# InfluxDB 2.x — downloaded from the official source (no prebuilt hub image).
ARG INFLUXDB_VERSION=2.7.10
RUN wget -q "https://dl.influxdata.com/influxdb/releases/influxdb2-${INFLUXDB_VERSION}-linux-amd64.tar.gz" \
        -O /tmp/influxdb.tar.gz \
    && tar -xzf /tmp/influxdb.tar.gz -C /opt \
    && mv /opt/influxdb2-${INFLUXDB_VERSION}-linux-amd64/influxd /usr/local/bin/influxd \
    && rm -rf /tmp/influxdb.tar.gz /opt/influxdb2-${INFLUXDB_VERSION}-linux-amd64

EXPOSE 8086

ENTRYPOINT ["/usr/local/bin/influxd"]
