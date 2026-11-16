#include "test_framework.h"

t_stats	g_stats = {0, 0, 0, 0, 0, 0, 0};

int	main(int argc, char **argv)
{
	const char	*mode = (argc > 1) ? argv[1] : "STANDARD";

	printf(CLR_BOLD CLR_CYAN "=== TESTS UNITAIRES & CAS LIMITES SUR LES 43 FONCTIONS (%s) ===" CLR_RESET "\n", mode);
	run_part1_tests();
	run_part2_tests();
	run_part3_tests();

	printf("\n" CLR_BOLD "Bilan (%s) : Fonctions %s%d/43 PASS" CLR_RESET
		CLR_BOLD " | Assertions %s%d/%d OK" CLR_RESET "\n\n",
		mode,
		(g_stats.fn_passed == 43 && g_stats.fn_failed == 0) ? CLR_GREEN : CLR_RED,
		g_stats.fn_passed,
		(g_stats.failed_assertions == 0) ? CLR_GREEN : CLR_RED,
		g_stats.total_assertions - g_stats.failed_assertions,
		g_stats.total_assertions);

	if (g_stats.fn_index != 43 || g_stats.fn_failed != 0 || g_stats.failed_assertions != 0)
		return (1);
	return (0);
}
