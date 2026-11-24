# ============================================================================ #
#  Tests — FTPS container (proftpd + SSL)                                    #
# ============================================================================ #
from lib import check, expect, read_file


def test_proftpd_listens_21():
    conf = read_file("srcs/ftp/proftpd.conf")
    check("Port               21" in conf or "Port 21" in conf,
          "proftpd: écoute le port 21")
    check("Listen" in conf, "proftpd: Listen activé")


def test_proftpd_ssl_ftps():
    conf = read_file("srcs/ftp/proftpd.conf")
    check("TLSEngine" in conf, "proftpd: TLS activé (FTPS)")
    check("TLSProtocol" in conf, "proftpd: définit TLSProtocol")
    check("TLSCertificateFile" in conf, "proftpd: certificate TLS")
    check("TLSRequired" in conf, "proftpd: TLS requis (FTPS)")


def test_entrypoint_generates_cert_and_user():
    entry = read_file("srcs/ftp/entrypoint.sh")
    check("openssl req" in entry, "entrypoint FTP: génère un certificat (openssl)")
    check("adduser" in entry or "useradd" in entry,
          "entrypoint FTP: crée l'utilisateur")
    check("chpasswd" in entry, "entrypoint FTP: mot de passe via Secret")
    check("exec proftpd" in entry, "entrypoint FTP: lance proftpd en foreground")


def test_dockerfile_installs_proftpd_and_openssl():
    dockerfile = read_file("srcs/ftp/Dockerfile")
    check("proftpd" in dockerfile, "Dockerfile ftp: installe proftpd")
    check("openssl" in dockerfile, "Dockerfile ftp: installe openssl (TLS)")
    check("EXPOSE 21" in dockerfile, "Dockerfile ftp: expose 21")
