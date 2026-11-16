from django.http import HttpResponse
from django.shortcuts import render
from sql_helper import get_connection, create_table

TABLE_SQL = """
CREATE TABLE IF NOT EXISTS ex00_movies (
    title VARCHAR(64) NOT NULL UNIQUE,
    episode_nb INTEGER PRIMARY KEY,
    opening_crawl TEXT,
    director VARCHAR(32) NOT NULL,
    producer VARCHAR(128) NOT NULL,
    release_date DATE NOT NULL
)
"""

def init(request):
    try:
        conn = get_connection()
        create_table(conn, TABLE_SQL)
        return HttpResponse("OK")
    except Exception as e:
        return HttpResponse(f"Error: {e}", status=500)
