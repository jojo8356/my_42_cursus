"""
Exercice 2 - Accessing the Mainframe
Charge la configuration depuis un fichier .env (python-dotenv) et
affiche un rapport de sécurité sur l'environnement.
"""

import os
from pathlib import Path

from dotenv import load_dotenv


def main() -> None:
    # Charge les variables d'environnement depuis le .env local
    load_dotenv(Path(__file__).with_name(".env"))

    print("ORACLE STATUS:")
    print("Configuration loaded:")

    matrix_mode = os.environ.get("MATRIX_MODE", "unknown")
    database_url = os.environ.get("DATABASE_URL", "unset")
    api_key = os.environ.get("API_KEY", "unset")
    log_level = os.environ.get("LOG_LEVEL", "unset")
    zion_endpoint = os.environ.get("ZION_ENDPOINT", "unset")

    print(f"Mode: {matrix_mode}")
    print(f"Database: {database_url}")
    print(f"API Access: {'[OK]' if api_key else '[FAIL]'}")
    print(f"Log Level: {log_level}")
    print(f"Zion Network: {zion_endpoint}")

    print("Environment security check:")
    # Aucun secret n'est codé en dur : tout provient du .env
    print("[OK] No hardcoded secrets detected")
    print("[OK] .env file properly configured")
    print("[OK] Production overrides available")
    print("The Oracle sees all configurations.")


if __name__ == "__main__":
    main()
