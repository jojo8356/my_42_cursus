from django.contrib.auth import get_user_model
from django.test import TestCase
from django.urls import reverse

from .models import Article, UserFavouriteArticle


class ArticlesViewsTestCase(TestCase):
    """Tests des regles d'acces des vues de l'exercice 06."""

    @classmethod
    def setUpTestData(cls):
        User = get_user_model()
        cls.author = User.objects.create_user('author', password='password123')
        cls.reader = User.objects.create_user('reader', password='password123')
        cls.article = Article.objects.create(
            title='Article de test',
            author=cls.author,
            synopsis='Un resume de test',
            content='Le contenu de test',
        )

    def test_favourites_view_is_only_accessible_to_logged_in_users(self):
        """La vue favourites exige d'etre connecte."""
        response = self.client.get(reverse('favourites'))
        self.assertEqual(response.status_code, 302)
        self.client.login(username='reader', password='password123')
        response = self.client.get(reverse('favourites'))
        self.assertEqual(response.status_code, 200)

    def test_publications_view_is_only_accessible_to_logged_in_users(self):
        """La vue publications exige d'etre connecte."""
        response = self.client.get(reverse('publications'))
        self.assertEqual(response.status_code, 302)
        self.client.login(username='reader', password='password123')
        response = self.client.get(reverse('publications'))
        self.assertEqual(response.status_code, 200)

    def test_publish_view_is_only_accessible_to_logged_in_users(self):
        """La vue publish exige d'etre connecte."""
        response = self.client.get(reverse('publish'))
        self.assertEqual(response.status_code, 302)
        self.client.login(username='reader', password='password123')
        response = self.client.get(reverse('publish'))
        self.assertEqual(response.status_code, 200)

    def test_favourites_template_is_only_rendered_to_logged_in_users(self):
        """Le template favourites n'est pas rendu pour un anonyme."""
        response = self.client.get(reverse('favourites'))
        self.assertTemplateNotUsed(response, 'articles_app/favourites.html')
        self.client.login(username='reader', password='password123')
        response = self.client.get(reverse('favourites'))
        self.assertTemplateUsed(response, 'articles_app/favourites.html')

    def test_publications_template_is_only_rendered_to_logged_in_users(self):
        """Le template publications n'est pas rendu pour un anonyme."""
        response = self.client.get(reverse('publications'))
        self.assertTemplateNotUsed(response, 'articles_app/publications.html')
        self.client.login(username='reader', password='password123')
        response = self.client.get(reverse('publications'))
        self.assertTemplateUsed(response, 'articles_app/publications.html')

    def test_publish_template_is_only_rendered_to_logged_in_users(self):
        """Le template publish n'est pas rendu pour un anonyme."""
        response = self.client.get(reverse('publish'))
        self.assertTemplateNotUsed(response, 'articles_app/publish.html')
        self.client.login(username='reader', password='password123')
        response = self.client.get(reverse('publish'))
        self.assertTemplateUsed(response, 'articles_app/publish.html')

    def test_logged_in_user_cannot_access_the_register_form(self):
        """Un utilisateur connecte n'a pas acces au formulaire d'inscription."""
        self.client.login(username='reader', password='password123')
        response = self.client.get(reverse('register'))
        self.assertEqual(response.status_code, 302)

    def test_logged_in_user_cannot_post_to_the_register_form(self):
        """Un utilisateur connecte ne peut pas creer de compte via register."""
        self.client.login(username='reader', password='password123')
        response = self.client.post(reverse('register'), {
            'username': 'sneaky',
            'password1': 'password123',
            'password2': 'password123',
        })
        self.assertEqual(response.status_code, 302)
        User = get_user_model()
        self.assertFalse(User.objects.filter(username='sneaky').exists())

    def test_a_user_cannot_favourite_the_same_article_twice(self):
        """Un meme article ne peut pas etre ajoute deux fois en favori."""
        self.client.login(username='reader', password='password123')
        url = reverse('add_favourite', args=[self.article.pk])
        self.client.post(url, {'article': self.article.pk})
        self.assertTrue(UserFavouriteArticle.objects.filter(
            user=self.reader, article=self.article).exists())
        response = self.client.post(url, {'article': self.article.pk})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(UserFavouriteArticle.objects.filter(
            user=self.reader, article=self.article).count(), 1)
