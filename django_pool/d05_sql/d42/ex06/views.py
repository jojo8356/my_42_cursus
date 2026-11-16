from django.http import HttpResponse
from django.shortcuts import render
from sql_helper import get_connection, create_table, execute_sql, fetch_all, MOVIES_DATA
from datetime import datetime

TABLE_SQL = """
CREATE TABLE IF NOT EXISTS ex06_movies (
    title VARCHAR(64) NOT NULL UNIQUE,
    episode_nb INTEGER PRIMARY KEY,
    opening_crawl TEXT,
    director VARCHAR(32) NOT NULL,
    producer VARCHAR(128) NOT NULL,
    release_date DATE NOT NULL,
    created TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
)
"""

INSERT_SQL = """
INSERT OR REPLACE INTO ex06_movies (title, episode_nb, director, producer, release_date, created, updated)
VALUES (?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
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
        rows = fetch_all(conn, "SELECT * FROM ex06_movies")
    except Exception:
        return HttpResponse("No data available")
    if not rows:
        return HttpResponse("No data available")
    rows = [dict(row) if hasattr(row, 'keys') else row for row in rows]
    return render(request, 'ex06/display.html', {'movies': rows})

def update(request):
    conn = get_connection()
    if request.method == 'POST':
        title = request.POST.get('title', '')
        opening_crawl = request.POST.get('opening_crawl', '')
        if title:
            try:
                execute_sql(conn, "UPDATE ex06_movies SET opening_crawl = ?, updated = CURRENT_TIMESTAMP WHERE title = ?", (opening_crawl, title))
            except Exception:
                return HttpResponse("No data available")
    try:
        rows = fetch_all(conn, "SELECT title FROM ex06_movies")
    except Exception:
        return HttpResponse("No data available")
    titles = [row['title'] if hasattr(row, 'keys') else row[0] for row in rows]
    if not titles:
        return HttpResponse("No data available")
    return render(request, 'ex06/update.html', {'titles': titles})
