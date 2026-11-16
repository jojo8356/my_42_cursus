from django.http import HttpResponse
from django.shortcuts import render
from sql_helper import get_connection, create_table, execute_sql, fetch_all, MOVIES_DATA

TABLE_SQL = """
CREATE TABLE IF NOT EXISTS ex02_movies (
    title VARCHAR(64) NOT NULL UNIQUE,
    episode_nb INTEGER PRIMARY KEY,
    opening_crawl TEXT,
    director VARCHAR(32) NOT NULL,
    producer VARCHAR(128) NOT NULL,
    release_date DATE NOT NULL
)
"""

INSERT_SQL = """
INSERT INTO ex02_movies (title, episode_nb, director, producer, release_date)
VALUES (?, ?, ?, ?, ?)
"""

def init(request):
    try:
        conn = get_connection()
        create_table(conn, TABLE_SQL)
        return HttpResponse("OK")
    except Exception as e:
        return HttpResponse(f"Error: {e}", status=500)

def populate(request):
    conn = get_connection()
    results = []
    for episode_nb, title, director, producer, release_date in MOVIES_DATA:
        try:
            execute_sql(conn, INSERT_SQL, (title, episode_nb, director, producer, release_date))
            results.append("OK")
        except Exception as e:
            results.append(f"{title}: {e}")
    return HttpResponse("\n".join(results))

def display(request):
    conn = get_connection()
    try:
        rows = fetch_all(conn, "SELECT * FROM ex02_movies")
    except Exception:
        return HttpResponse("No data available")
    if not rows:
        return HttpResponse("No data available")
    rows = [dict(row) if hasattr(row, 'keys') else row for row in rows]
    return render(request, 'ex02/display.html', {'movies': rows})
