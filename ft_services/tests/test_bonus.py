# ============================================================================ #
#  Tests — Bonus services (Grafana, InfluxDB, Telegraf, SSH)                 #
# ============================================================================ #
from lib import check, expect, read_file, path_exists


def test_all_bonus_dockerfiles_present():
    for rel in (
        "srcs/bonus/grafana/Dockerfile",
        "srcs/bonus/influxdb/Dockerfile",
        "srcs/bonus/telegraf/Dockerfile",
        "srcs/bonus/ssh/Dockerfile",
    ):
        check(path_exists(rel), f"Dockerfile bonus présent: {rel}")


def test_grafana_dockerfile():
    df = read_file("srcs/bonus/grafana/Dockerfile")
    check("grafana" in df, "Grafana: image Grafana")
    check("EXPOSE 3000" in df, "Grafana: expose 3000")


def test_influxdb_dockerfile():
    df = read_file("srcs/bonus/influxdb/Dockerfile")
    check("influxd" in df, "InfluxDB: binary influxd")
    check("EXPOSE 8086" in df, "InfluxDB: expose 8086")


def test_telegraf_dockerfile_and_config():
    df = read_file("srcs/bonus/telegraf/Dockerfile")
    check("telegraf" in df, "Telegraf: binary telegraf")
    conf = read_file("srcs/bonus/telegraf/telegraf.conf")
    check("[[outputs.influxdb]]" in conf,
          "Telegraf: output vers InfluxDB")
    check("influxdb:8086" in conf, "Telegraf: pointe vers influxdb:8086")
    check("[[inputs.docker]]" in conf, "Telegraf: collecte docker")


def test_ssh_dockerfile_and_entrypoint():
    df = read_file("srcs/bonus/ssh/Dockerfile")
    check("openssh" in df, "SSH: installe openssh")
    check("ssh-keygen" in df, "SSH: génère les host keys")
    check("EXPOSE 22" in df, "SSH: expose 22")
    entry = read_file("srcs/bonus/ssh/entrypoint.sh")
    check("sshd" in entry, "SSH entrypoint: lance sshd")
    check("exec /usr/sbin/sshd" in entry or "exec sshd" in entry,
          "SSH entrypoint: exec sshd en foreground")


def test_bonus_manifests_present():
    for rel in (
        "yaml/bonus/grafana.yaml",
        "yaml/bonus/influxdb.yaml",
        "yaml/bonus/telegraf.yaml",
        "yaml/bonus/ssh.yaml",
    ):
        check(path_exists(rel), f"manifest bonus présent: {rel}")
