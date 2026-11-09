#include "../libft/libft.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLR_GREEN "\033[0;32m"
#define CLR_RED   "\033[0;31m"
#define CLR_CYAN  "\033[0;36m"
#define CLR_BOLD  "\033[1m"
#define CLR_RESET "\033[0m"

#define MAX_LIVE 4096

void	*__real_malloc(size_t size);
void	__real_free(void *ptr);

static int		g_tracking = 0;
static int		g_call_idx = 0;
static int		g_fail_at = 0;
static int		g_live_count = 0;
static int		g_invalid_free = 0;
static void		*g_live[MAX_LIVE];

void	*__wrap_malloc(size_t size)
{
	void	*p;

	if (!g_tracking)
		return (__real_malloc(size));
	g_call_idx++;
	if (g_fail_at > 0 && g_call_idx == g_fail_at)
		return (NULL);
	p = __real_malloc(size);
	if (p && g_live_count < MAX_LIVE)
		g_live[g_live_count++] = p;
	return (p);
}

void	__wrap_free(void *ptr)
{
	if (!ptr)
		return ;
	if (!g_tracking)
	{
		__real_free(ptr);
		return ;
	}
	for (int i = 0; i < g_live_count; i++)
	{
		if (g_live[i] == ptr)
		{
			g_live[i] = g_live[--g_live_count];
			__real_free(ptr);
			return ;
		}
	}
	g_invalid_free++;
}

static void	reset_tracker(int fail_at)
{
	g_call_idx = 0;
	g_fail_at = fail_at;
	g_live_count = 0;
	g_invalid_free = 0;
	g_tracking = 1;
}

static void	stop_tracker(void)
{
	g_tracking = 0;
}

static char	map_char_fn(unsigned int i, char c)
{
	return ((char)(c + (i % 3)));
}

static void	*map_list_fn(void *content)
{
	int	*p = (int *)malloc(sizeof(int));

	if (!p)
		return (NULL);
	*p = *(int *)content * 10;
	return (p);
}

static void	del_list_fn(void *content)
{
	free(content);
}

static void	free_split_res(char **arr)
{
	if (!arr)
		return ;
	for (size_t i = 0; arr[i]; i++)
		free(arr[i]);
	free(arr);
}

int	main(void)
{
	int	passed = 0;
	int	failed = 0;

	printf(CLR_BOLD CLR_CYAN "=== PHASE 4 : INJECTION SYSTEMATIQUE D'ECHECS MALLOC & ZERO FUITE MEMOIRE ===" CLR_RESET "\n");

	/* 1. ft_calloc */
	{
		reset_tracker(1);
		void *p = ft_calloc(16, 4);
		int ok = (p == NULL && g_live_count == 0 && g_invalid_free == 0);
		stop_tracker();
		if (ok) { passed++; printf("  " CLR_GREEN "[PASS]" CLR_RESET " ft_calloc          (echec malloc #1 -> NULL, 0 fuite)\n"); }
		else { failed++; printf("  " CLR_RED "[FAIL]" CLR_RESET " ft_calloc\n"); }
	}

	/* 2. ft_strdup */
	{
		reset_tracker(1);
		char *p = ft_strdup("Ecole 42 Nice");
		int ok = (p == NULL && g_live_count == 0 && g_invalid_free == 0);
		stop_tracker();
		if (ok) { passed++; printf("  " CLR_GREEN "[PASS]" CLR_RESET " ft_strdup          (echec malloc #1 -> NULL, 0 fuite)\n"); }
		else { failed++; printf("  " CLR_RED "[FAIL]" CLR_RESET " ft_strdup\n"); }
	}

	/* 3. ft_substr */
	{
		reset_tracker(1);
		char *p = ft_substr("Ecole 42 Nice", 6, 2);
		int ok = (p == NULL && g_live_count == 0 && g_invalid_free == 0);
		stop_tracker();
		if (ok) { passed++; printf("  " CLR_GREEN "[PASS]" CLR_RESET " ft_substr          (echec malloc #1 -> NULL, 0 fuite)\n"); }
		else { failed++; printf("  " CLR_RED "[FAIL]" CLR_RESET " ft_substr\n"); }
	}

	/* 4. ft_strjoin */
	{
		reset_tracker(1);
		char *p = ft_strjoin("Ecole ", "42");
		int ok = (p == NULL && g_live_count == 0 && g_invalid_free == 0);
		stop_tracker();
		if (ok) { passed++; printf("  " CLR_GREEN "[PASS]" CLR_RESET " ft_strjoin         (echec malloc #1 -> NULL, 0 fuite)\n"); }
		else { failed++; printf("  " CLR_RED "[FAIL]" CLR_RESET " ft_strjoin\n"); }
	}

	/* 5. ft_strtrim */
	{
		reset_tracker(1);
		char *p = ft_strtrim("   42 Nice   ", " ");
		int ok = (p == NULL && g_live_count == 0 && g_invalid_free == 0);
		stop_tracker();
		if (ok) { passed++; printf("  " CLR_GREEN "[PASS]" CLR_RESET " ft_strtrim         (echec malloc #1 -> NULL, 0 fuite)\n"); }
		else { failed++; printf("  " CLR_RED "[FAIL]" CLR_RESET " ft_strtrim\n"); }
	}

	/* 6. ft_split (6 allocations: 1 array + 5 words — fail at k = 1..6, succeed at k = 7) */
	{
		int ok = 1;
		for (int k = 1; k <= 6; k++)
		{
			reset_tracker(k);
			char **arr = ft_split("un deux trois quatre cinq", ' ');
			if (arr != NULL || g_live_count != 0 || g_invalid_free != 0)
				ok = 0;
			stop_tracker();
		}
		reset_tracker(0);
		char **arr = ft_split("un deux trois quatre cinq", ' ');
		if (!arr)
			ok = 0;
		free_split_res(arr);
		if (g_live_count != 0 || g_invalid_free != 0)
			ok = 0;
		stop_tracker();
		if (ok) { passed++; printf("  " CLR_GREEN "[PASS]" CLR_RESET " ft_split           (echecs malloc #1..#6 sur 5 mots -> NULL, 0 fuite)\n"); }
		else { failed++; printf("  " CLR_RED "[FAIL]" CLR_RESET " ft_split\n"); }
	}

	/* 7. ft_itoa */
	{
		reset_tracker(1);
		char *p1 = ft_itoa(-2147483648);
		int ok = (p1 == NULL && g_live_count == 0 && g_invalid_free == 0);
		stop_tracker();
		reset_tracker(1);
		char *p2 = ft_itoa(0);
		ok = ok && (p2 == NULL && g_live_count == 0 && g_invalid_free == 0);
		stop_tracker();
		if (ok) { passed++; printf("  " CLR_GREEN "[PASS]" CLR_RESET " ft_itoa            (echecs malloc sur 0 et INT_MIN -> NULL, 0 fuite)\n"); }
		else { failed++; printf("  " CLR_RED "[FAIL]" CLR_RESET " ft_itoa\n"); }
	}

	/* 8. ft_strmapi */
	{
		reset_tracker(1);
		char *p = ft_strmapi("42Nice", map_char_fn);
		int ok = (p == NULL && g_live_count == 0 && g_invalid_free == 0);
		stop_tracker();
		if (ok) { passed++; printf("  " CLR_GREEN "[PASS]" CLR_RESET " ft_strmapi         (echec malloc #1 -> NULL, 0 fuite)\n"); }
		else { failed++; printf("  " CLR_RED "[FAIL]" CLR_RESET " ft_strmapi\n"); }
	}

	/* 9. ft_lstnew */
	{
		int x = 42;
		reset_tracker(1);
		t_list *n = ft_lstnew(&x);
		int ok = (n == NULL && g_live_count == 0 && g_invalid_free == 0);
		stop_tracker();
		if (ok) { passed++; printf("  " CLR_GREEN "[PASS]" CLR_RESET " ft_lstnew          (echec malloc #1 -> NULL, 0 fuite)\n"); }
		else { failed++; printf("  " CLR_RED "[FAIL]" CLR_RESET " ft_lstnew\n"); }
	}

	/* 10. ft_lstmap (4 nodes: fails at node-alloc calls #2, #4, #6, #8 AND f-alloc calls #1, #3, #5, #7) */
	{
		int		vals[4] = {1, 2, 3, 4};
		t_list	*orig = NULL;
		int		ok = 1;

		for (int i = 0; i < 4; i++)
			ft_lstadd_back(&orig, ft_lstnew(&vals[i]));

		/* Failing EITHER f's malloc (#1, #3, #5, #7) OR ft_lstnew's malloc (#2, #4, #6, #8):
		   every failure k = 1..8 must free all partial allocations, return NULL, and keep orig intact */
		for (int k = 1; k <= 8; k++)
		{
			reset_tracker(k);
			t_list *m = ft_lstmap(orig, map_list_fn, del_list_fn);
			if (m != NULL || g_live_count != 0 || g_invalid_free != 0)
				ok = 0;
			stop_tracker();
			if (ft_lstsize(orig) != 4U || *(int *)orig->content != 1
				|| *(int *)ft_lstlast(orig)->content != 4)
				ok = 0;
		}

		ft_lstclear(&orig, NULL);
		if (ok) { passed++; printf("  " CLR_GREEN "[PASS]" CLR_RESET " ft_lstmap          (echecs malloc #1..#8 sur 4 maillons -> 0 fuite, liste source intacte)\n"); }
		else { failed++; printf("  " CLR_RED "[FAIL]" CLR_RESET " ft_lstmap\n"); }
	}

	printf("\n" CLR_BOLD "Bilan Phase 4 : " CLR_GREEN "%d/10 fonctions d'allocation OK" CLR_RESET
		", " CLR_RED "%d KO" CLR_RESET "\n\n", passed, failed);
	return (failed == 0 ? 0 : 1);
}
