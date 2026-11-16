# Exercice 02 : Requêter une API
# Dossier : ex02/ - Fichiers : request_wikipedia.py, requirement.txt
import sys
import re
import requests
import json

def clean_wiki_text(text):
    """Nettoie le formatage Wiki Markup du texte."""
    # Supprimer les balises
    text = re.sub(r'<[^>]+>', '', text)
    # Supprimer les redirections (== Titre ==)
    text = re.sub(r'==.*?==', '', text)
    # Supprimer les liens [texte|lien] ou [texte]
    text = re.sub(r'\[[^\]]*\]', '', text)
    # Supprimer les modèles {{...}}
    text = re.sub(r'\{\{.*?\}\}', '', text)
    # Réduire les espaces multiples
    text = re.sub(r'\s+', ' ', text).strip()
    return text

def search_wikipedia(query):
    """Requête l'API Wikipedia et retourne le texte net."""
    url = 'https://fr.wikipedia.org/w/api.php'
    params = {
        'action': 'query',
        'list': 'search',
        'srsearch': query,
        'format': 'json',
        'srlimit': 1,
        'srprop': ''
    }

    response = requests.get(url, params=params)
    if response.status_code != 200:
        return None, "Problème serveur"

    data = response.json()

    if 'query' not in data or 'search' not in data['query']:
        return None, "Information non trouvée"

    results = data['query']['search']
    if not results:
        return None, "Information non trouvée"

    page_title = results[0]['title']

    # Récupérer le contenu de l'article
    url2 = 'https://fr.wikipedia.org/w/api.php'
    params2 = {
        'action': 'query',
        'titles': page_title,
        'prop': 'extracts',
        'explaintext': 1,
        'format': 'json',
        'redirects': 1
    }

    response2 = requests.get(url2, params=params2)
    if response2.status_code != 200:
        return None, "Problème serveur"

    data2 = response2.json()
    pages = data2.get('query', {}).get('pages', {})
    if not pages:
        return None, "Information non trouvée"

    page = list(pages.values())[0]
    if 'extract' not in page:
        return None, "Information non trouvée"

    extract = page['extract']
    return extract, None

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 request_wikipedia.py \"<recherche>\"")
        return

    query = sys.argv[1]
    result, error = search_wikipedia(query)

    if error:
        print(f"Erreur: {error}")
        return

    # Nettoyer et écrire dans le fichier
    cleaned = clean_wiki_text(result)

    # Nom du fichier sans espaces
    filename = query.replace(' ', '_') + '.wiki'
    with open(filename, 'w') as f:
        f.write(cleaned)

    print(f"Résultat écrit dans {filename}")

if __name__ == '__main__':
    main()
