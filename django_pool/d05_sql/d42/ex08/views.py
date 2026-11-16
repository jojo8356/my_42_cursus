import csv
import os
from django.http import HttpResponse
from django.shortcuts import render
from sql_helper import get_connection, create_table, execute_sql, fetch_all

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

PLANETS_FIELDS = ['name', 'climate', 'diameter', 'orbital_period',
                 'population', 'rotation_period', 'surface_water', 'terrain']

PLANETS_SQL = """
CREATE TABLE IF NOT EXISTS ex08_planets (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL UNIQUE,
    climate VARCHAR(64),
    diameter INTEGER,
    orbital_period INTEGER,
    population BIGINT,
    rotation_period INTEGER,
    surface_water REAL,
    terrain VARCHAR(128)
)
"""

PEOPLE_SQL = """
CREATE TABLE IF NOT EXISTS ex08_people (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL UNIQUE,
    birth_year VARCHAR(32),
    gender VARCHAR(32),
    eye_color VARCHAR(32),
    hair_color VARCHAR(32),
    height INTEGER,
    mass REAL,
    homeworld VARCHAR(64) REFERENCES ex08_planets(name)
)
"""

def init(request):
    try:
        conn = get_connection()
        create_table(conn, PLANETS_SQL)
        create_table(conn, PEOPLE_SQL)
        return HttpResponse("OK")
    except Exception as e:
        return HttpResponse(f"Error: {e}", status=500)

def populate(request):
    conn = get_connection()
    results = []

    # Copier planets.csv
    planets_csv = os.path.join(BASE_DIR, 'planets.csv')
    try:
        with open(planets_csv, 'r', newline='') as f:
            reader = csv.reader(f, delimiter='\t')
            for row in reader:
                try:
                    values = [None if v == 'NULL' else v for v in row]
                    execute_sql(
                        conn,
                        "INSERT OR REPLACE INTO ex08_planets (name, climate, diameter, orbital_period, population, rotation_period, surface_water, terrain) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                        tuple(values)
                    )
                except Exception as e:
                    results.append(f"planets: {e}")
        results.append("OK")
    except Exception as e:
        results.append(f"planets: {e}")

    # Copier people.csv
    people_csv = os.path.join(BASE_DIR, 'people.csv')
    try:
        with open(people_csv, 'r', newline='') as f:
            reader = csv.reader(f, delimiter='\t')
            for row in reader:
                try:
                    values = [None if v == 'NULL' else v for v in row]
                    execute_sql(
                        conn,
                        "INSERT OR REPLACE INTO ex08_people (name, birth_year, gender, eye_color, hair_color, height, mass, homeworld) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                        tuple(values)
                    )
                except Exception as e:
                    results.append(f"people: {e}")
        results.append("OK")
    except Exception as e:
        results.append(f"people: {e}")

    return HttpResponse("\n".join(results))

def display(request):
    conn = get_connection()
    try:
        rows = fetch_all(
            conn,
            """
            SELECT ex08_people.name, ex08_people.homeworld, ex08_planets.climate
            FROM ex08_people
            JOIN ex08_planets ON ex08_people.homeworld = ex08_planets.name
            WHERE ex08_planets.climate LIKE '%windy%'
            ORDER BY ex08_people.name
            """
        )
    except Exception:
        return HttpResponse("No data available")
    if not rows:
        return HttpResponse("No data available")
    rows = [dict(row) if hasattr(row, 'keys') else row for row in rows]
    return render(request, 'ex08/display.html', {'people': rows})
