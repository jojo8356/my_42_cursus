# Exercice 03 : Roads to philosophy
# Dossier : ex03/ - Fichiers : roads_to_philosophy.py, requirement.txt
import sys
import requests
from bs4 import BeautifulSoup


def get_wikipedia_page(url):
    """Récupère une page Wikipedia en gérant les redirections."""
    try:
        response = requests.get(url, headers={'User-Agent': 'Mozilla/5.0'})
        response.raise_for_status()
        soup = BeautifulSoup(response.text, 'html.parser')
        return soup
    except requests.RequestException as e:
        print(f"Erreur de requête: {e}")
        return None


def get_article_info(soup):
    """Récupère le titre et le premier lien valide de l'introduction."""
    # Titre principal
    title_tag = soup.find('h1')
    if not title_tag:
        return None, None
    title = title_tag.get_text(strip=True)

    # Premier lien dans le paragraphe d'introduction
    # Trouver le premier <p> qui contient des liens
    for p in soup.find_all('p'):
        links = p.find_all('a', href=True)
        for link in links:
            href = link.get('href', '')
            # Ignorer les liens externes, les sections d'aide, les redi
            # Redirections internes
            if href.startswith('/wiki/'):
                # Ignorer les liens en italique ou entre parenthèses
                # On ignore aussi les liens vers les pages d'aide ou de spéciale
                if ':' in href.split('/wiki/')[1].split('?')[0]:
                    continue
                # Ignorer les liens qui sont en italique (class mw-redirectedfrom, etc)
                if link.find_parent('i'):
                    continue
                # Ignorer les liens vers des sections
                if '#' in href:
                    continue
                return title, href
    return title, None


def get_redirect_target(soup):
    """Trouve une redirection (lien rouge avec tooltip)."""
    # Les redirections sont des liens avec title="..." contenant "redirect"
    for link in soup.find_all('a', href=True):
        href = link.get('href', '')
        title = link.get('title', '')
        if 'redirect' in title.lower() and href.startswith('/wiki/'):
            return href
    return None


def follow_road(query):
    """Suit la chaîne de liens jusqu'à Philosophie ou impasse."""
    # URL de recherche Wikipedia
    search_url = f"https://en.wikipedia.org/wiki/{query.replace(' ', '_')}"

    soup = get_wikipedia_page(search_url)
    if soup is None:
        return None, "Erreur de connexion ou de requête"

    # Gestion des redirections
    redirect_href = get_redirect_target(soup)
    if redirect_href:
        soup = get_wikipedia_page(f"https://en.wikipedia.org{redirect_href}")
        if soup is None:
            return None, "Erreur de connexion"

    visited = []

    while True:
        title, next_link = get_article_info(soup)
        if not title:
            return visited, "It's a dead end !"

        visited.append(title)

        # Si on est sur Philosophie
        if title.lower() == "philosophy":
            count = len(visited) - 1  # Ne pas compter Philosophie ?
            # En fait, l'énoncé veut le compte total des articles visités
            count = len(visited)
            return visited, f"{count} roads from {query} to philosophy !"

        if next_link is None:
            return visited, "It's a dead end !"

        # Si le lien est déjà visité
        next_url = f"https://en.wikipedia.org{next_link}"
        next_soup = get_wikipedia_page(next_url)
        if next_soup is None:
            return visited, "Erreur de connexion"

        next_title, _ = get_article_info(next_soup)
        if next_title and next_title in visited:
            return visited, "It leads to an infinite loop !"

        soup = next_soup


def main():
    if len(sys.argv) != 2:
        print("Usage: python3 roads_to_philosophy.py \"<recherche>\"")
        return

    query = sys.argv[1]
    visited, result = follow_road(query)

    if result.startswith("Erreur"):
        print(f"Erreur: {result}")
        return

    for article in visited:
        print(article)
    print(result)


if __name__ == '__main__':
    main()
