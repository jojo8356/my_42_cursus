# ============================================================================ #
#  Tests — Makefile & .env                                                   #
# ============================================================================ #
from lib import check, expect, read_file


def _makefile():
    return read_file("Makefile")


def _env():
    return read_file("srcs/.env")


def test_makefile_uses_docker_compose():
    """The Makefile must drive docker compose."""
    mk = _makefile()
    check("docker compose" in mk or "COMPOSE" in mk,
          "Makefile utilise docker compose")


def test_makefile_has_core_targets():
    """Mandatory targets: up, down, build, clean, re."""
    mk = _makefile()
    for target in ("up", "down", "build", "clean", "re"):
        check(f"{target}:" in mk, f"Makefile a la cible '{target}:'")


def test_makefile_re_is_clean_build_up():
    """'re' must be clean + build + up."""
    mk = _makefile()
    check("re: clean build up" in mk or "re: clean build" in mk,
          "cible 're' = clean build up")


def test_makefile_clean_removes_volumes():
    """clean must remove volumes to fully reset state."""
    mk = _makefile()
    check("-v" in mk or "--volumes" in mk, "cible clean supprime les volumes")


def test_env_file_exists_and_has_required_vars():
    """The .env must define domain, admin user, DB settings."""
    env = _env()
    for var in ("DOMAIN_NAME", "WP_ADMIN", "WP_USER", "DB_HOST", "DB_USER", "DB_DATABASE"):
        check(f"{var}=" in env, f".env définit {var}")


def test_env_admin_not_admin():
    """The .env admin name must not be 'admin'/'administrator'."""
    env = _env()
    for line in env.splitlines():
        if line.startswith("WP_ADMIN="):
            admin = line.split("=", 1)[1].strip()
            check(admin.lower() not in ("admin", "administrator"),
                  ".env: WP_ADMIN n'est pas 'admin'/'administrator'", admin)


def test_env_does_not_leak_passwords():
    """The .env must not contain real passwords (secrets live in files)."""
    env = _env()
    # Only count real assignments, not comments that mention the variable.
    assignments = [
        line for line in env.splitlines()
        if line.strip() and not line.strip().startswith("#")
        and ("WP_ADMIN_PASSWORD" in line or "DB_PASSWORD" in line)
    ]
    expect(not assignments,
           ".env ne contient pas de mot de passe en dur",
           f"assignements trouvés: {assignments}")


def test_secrets_gitignored():
    """Real secret files must be git-ignored."""
    gitignore = read_file(".gitignore")
    check("secrets/*.txt" in gitignore, ".gitignore ignore secrets/*.txt")
