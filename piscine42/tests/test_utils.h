/* Micro-framework de test pour la piscine (fichier d'appui, jamais rendu). */
#ifndef TEST_UTILS_H
# define TEST_UTILS_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>

static int	g_tests_passed = 0;
static int	g_tests_failed = 0;

/* Verifie une condition et affiche le resultat du test. */
static void	check(int cond, const char *name)
{
	if (cond)
	{
		g_tests_passed++;
		printf("  \033[32mPASS\033[0m %s\n", name);
	}
	else
	{
		g_tests_failed++;
		printf("  \033[31mFAIL\033[0m %s\n", name);
	}
}

/* Compare deux chaines (NULL accepte) et affiche le resultat. */
static void	check_str(const char *got, const char *want, const char *name)
{
	int	ok;

	if (got == NULL || want == NULL)
		ok = (got == want);
	else
		ok = (strcmp(got, want) == 0);
	if (!ok)
		printf("        obtenu=\"%s\" attendu=\"%s\"\n",
			got ? got : "(null)", want ? want : "(null)");
	check(ok, name);
}

/* Compare deux entiers et affiche le resultat. */
static void	check_int(int got, int want, const char *name)
{
	if (got != want)
		printf("        obtenu=%d attendu=%d\n", got, want);
	check(got == want, name);
}

/* Affiche le bilan et quitte avec le code d'erreur correspondant. */
static int	test_summary(const char *suite)
{
	printf("%s : %d reussi(s), %d echec(s)\n",
		suite, g_tests_passed, g_tests_failed);
	return (g_tests_failed == 0);
}

#endif
