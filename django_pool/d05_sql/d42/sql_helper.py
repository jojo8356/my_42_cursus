"""
Helper pour les exercices SQL.
En soutenance, psycopg2 est utilisé avec PostgreSQL.
Pour les tests ici, sqlite3 est utilisé en fallback.
"""
import os

USE_POSTGRESQL = os.environ.get('USE_POSTGRESQL', 'false').lower() == 'true'

if USE_POSTGRESQL:
    import psycopg2

    def get_connection():
        return psycopg2.connect(
            dbname='formationdjango',
            user='djangouser',
            password='secret',
            host='localhost',
            port='5432'
        )

    def create_table(conn, sql):
        """Crée une table via SQL."""
        with conn.cursor() as cur:
            cur.execute(sql)
        conn.commit()

    def execute_sql(conn, sql, params=None):
        """Exécute une requête SQL."""
        with conn.cursor() as cur:
            if params:
                cur.execute(sql, params)
            else:
                cur.execute(sql)
        conn.commit()

    def fetch_all(conn, sql, params=None):
        """Récupère toutes les lignes."""
        with conn.cursor() as cur:
            if params:
                cur.execute(sql, params)
            else:
                cur.execute(sql)
            return cur.fetchall()

    def get_columns(conn, table_name):
        """Récupère les colonnes d'une table."""
        with conn.cursor() as cur:
            cur.execute(f"SELECT column_name, data_type FROM information_schema.columns WHERE table_name = %s", (table_name,))
            return cur.fetchall()

else:
    import sqlite3

    DB_PATH = os.path.join(os.path.dirname(__file__), 'db.sqlite3')

    def get_connection():
        conn = sqlite3.connect(DB_PATH)
        conn.row_factory = sqlite3.Row
        return conn

    def create_table(conn, sql):
        conn.execute(sql)
        conn.commit()

    def execute_sql(conn, sql, params=None):
        if params:
            conn.execute(sql, params)
        else:
            conn.execute(sql)
        conn.commit()

    def fetch_all(conn, sql, params=None):
        if params:
            cur = conn.execute(sql, params)
        else:
            cur = conn.execute(sql)
        return cur.fetchall()

    def get_columns(conn, table_name):
        cur = conn.execute(f"PRAGMA table_info({table_name})")
        return cur.fetchall()

MOVIES_DATA = [
    (1, "The Phantom Menace", "George Lucas", "Rick McCallum", "1999-05-19"),
    (2, "Attack of the Clones", "George Lucas", "Rick McCallum", "2002-05-16"),
    (3, "Revenge of the Sith", "George Lucas", "Rick McCallum", "2005-05-19"),
    (4, "A New Hope", "George Lucas", "Gary Kurtz, Rick McCallum", "1977-05-25"),
    (5, "The Empire Strikes Back", "Irvin Kershner", "Gary Kurtz, Rick McCallum", "1980-05-17"),
    (6, "Return of the Jedi", "Richard Marquand", "Howard G. Kazanjian, George Lucas, Rick McCallum", "1983-05-25"),
    (7, "The Force Awakens", "J. J. Abrams", "Kathleen Kennedy, J. J. Abrams, Bryan Burk", "2015-12-11"),
]
