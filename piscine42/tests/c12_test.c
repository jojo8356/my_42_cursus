/* Tests complets du jour C12 (listes chainees). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../c12/ft_list.h"

void	ft_list_push_front(t_list **begin_list, void *data);
int		ft_list_size(t_list *begin_list);
t_list	*ft_list_last(t_list *begin_list);
void	ft_list_push_back(t_list **begin_list, void *data);
t_list	*ft_list_push_strs(int size, char **strs);
void	ft_list_clear(t_list *begin_list, void (*free_fct)(void *));
t_list	*ft_list_at(t_list *begin_list, unsigned int idx);
void	ft_list_reverse(t_list **begin_list);
void	ft_list_foreach(t_list *begin_list, void (*f)(void *));
void	ft_list_foreach_if(t_list *begin_list, void (*f)(void *),
			void *data_ref, int (*cmp)());
t_list	*ft_list_find(t_list *begin_list, void *data_ref, int (*cmp)());
void	ft_list_remove_if(t_list **begin_list, void *data_ref,
			int (*cmp)(), void (*free_fct)(void *));
void	ft_list_merge(t_list **begin_list1, t_list *begin_list2);
void	ft_list_sort(t_list **begin_list, int (*cmp)());
void	ft_list_reverse_fun(t_list *begin_list);
void	ft_sorted_list_insert(t_list **begin_list, void *data, int (*cmp)());
void	ft_sorted_list_merge(t_list **begin_list1, t_list *begin_list2,
			int (*cmp)());

static int	g_pass = 0;
static int	g_fail = 0;
static int	g_sum = 0;

static void	check(int cond, const char *name)
{
	if (cond)
	{
		g_pass++;
		printf("  PASS %s\n", name);
	}
	else
	{
		g_fail++;
		printf("  FAIL %s\n", name);
	}
}

static int	cmp_str(void *a, void *b)
{
	return (strcmp((char *)a, (char *)b));
}

static int	cmp_int_ptr(void *a, void *b)
{
	return (*(int *)a - *(int *)b);
}

static void	add_len(void *data)
{
	g_sum += (int)strlen((char *)data);
}

static t_list	*make_list(char **strs, int n)
{
	t_list	*lst;
	int		i;

	lst = 0;
	i = n - 1;
	while (i >= 0)
	{
		ft_list_push_front(&lst, strs[i]);
		i--;
	}
	return (lst);
}

int	main(void)
{
	t_list	*lst;
	t_list	*l2;
	t_list	*e;
	char	*strs[5] = {"un", "deux", "trois", "quatre", NULL};

	printf("== C12 ==\n");
	e = ft_create_elem("data");
	check(e != NULL && e->data == (void *)"data" && e->next == NULL,
		"ex00 create_elem");
	free(e);
	lst = make_list(strs, 3);
	check(ft_list_size(lst) == 3, "ex02 list_size == 3");
	check(ft_list_last(lst) != NULL && ft_list_last(lst)->data == (void *)"trois",
		"ex03 list_last est 'trois'");
	ft_list_push_back(&lst, "zero");
	check(ft_list_last(lst)->data == (void *)"zero", "ex04 push_back a la fin");
	check(ft_list_size(lst) == 4, "ex04 taille apres push_back");
	ft_list_push_front(&lst, "debut");
	check(lst->data == (void *)"debut" && ft_list_size(lst) == 5,
		"ex01 push_front au debut");
	ft_list_clear(lst, NULL);
	lst = ft_list_push_strs(3, strs);
	check(lst != NULL && lst->data == (void *)"trois"
		&& ft_list_last(lst)->data == (void *)"un",
		"ex05 push_strs : premier en fin");
	check(ft_list_size(lst) == 3, "ex05 taille == 3");
	check(ft_list_at(lst, 0) == lst, "ex07 list_at(0)");
	check(ft_list_at(lst, 2)->data == (void *)"un", "ex07 list_at(2)");
	check(ft_list_at(lst, 3) == NULL, "ex07 list_at(3) -> NULL");
	check(ft_list_at(lst, 42) == NULL, "ex07 list_at(42) -> NULL");
	ft_list_reverse(&lst);
	check(lst->data == (void *)"un" && lst->next->data == (void *)"deux"
		&& ft_list_last(lst)->data == (void *)"trois", "ex08 list_reverse");
	ft_list_reverse(&lst);
	check(lst->data == (void *)"trois", "ex08 reverse deux fois = initial");
	g_sum = 0;
	ft_list_foreach(lst, add_len);
	check(g_sum == 11, "ex09 foreach (2+4+5 = 11 caracteres)");
	g_sum = 0;
	ft_list_foreach_if(lst, add_len, (void *)"trois", cmp_str);
	check(g_sum == 5, "ex10 foreach_if sur 'trois' seul");
	check(ft_list_find(lst, (void *)"deux", cmp_str) == lst->next,
		"ex11 find 'deux'");
	check(ft_list_find(lst, (void *)"absent", cmp_str) == NULL,
		"ex11 find absent -> NULL");
	ft_list_remove_if(&lst, (void *)"deux", cmp_str, NULL);
	check(ft_list_size(lst) == 2 && lst->next->data == (void *)"un",
		"ex12 remove_if retire 'deux'");
	ft_list_remove_if(&lst, (void *)"trois", cmp_str, NULL);
	check(ft_list_size(lst) == 1, "ex12 remove_if en tete");
	ft_list_remove_if(&lst, (void *)"absent", cmp_str, NULL);
	check(ft_list_size(lst) == 1, "ex12 remove_if element absent");
	ft_list_clear(lst, NULL);
	lst = ft_list_push_strs(2, strs);
	l2 = 0;
	ft_list_push_back(&l2, "quatre");
	ft_list_push_back(&l2, "cinq");
	ft_list_merge(&lst, l2);
	check(ft_list_size(lst) == 4 && ft_list_last(lst)->data == (void *)"cinq",
		"ex13 merge concatene");
	ft_list_clear(lst, NULL);
	{
		int		a = 3, b = 1, c = 2;
		t_list	*s = 0;

		ft_list_push_back(&s, &a);
		ft_list_push_back(&s, &c);
		ft_list_push_back(&s, &b);
		ft_list_sort(&s, cmp_int_ptr);
		check(*(int *)s->data == 1 && *(int *)s->next->data == 2
			&& *(int *)ft_list_last(s)->data == 3, "ex14 list_sort entiers");
		ft_list_reverse_fun(s);
		check(*(int *)s->data == 3 && *(int *)s->next->data == 2
			&& *(int *)ft_list_last(s)->data == 1,
			"ex15 reverse_fun (pointeur de tete conserve)");
		check(ft_list_size(s) == 3, "ex15 taille inchangee");
		ft_list_clear(s, NULL);
	}
	{
		t_list	*s = 0;
		int		a = 10, b = 20, c = 30, d = 25;

		ft_sorted_list_insert(&s, &b, cmp_int_ptr);
		ft_sorted_list_insert(&s, &a, cmp_int_ptr);
		ft_sorted_list_insert(&s, &c, cmp_int_ptr);
		ft_sorted_list_insert(&s, &d, cmp_int_ptr);
		check(*(int *)s->data == 10 && *(int *)s->next->data == 20
			&& *(int *)s->next->next->data == 25
			&& *(int *)ft_list_last(s)->data == 30,
			"ex16 sorted_list_insert");
		{
			t_list	*s2 = 0;
			int		x = 5, y = 22, z = 40;

			ft_list_push_back(&s2, &x);
			ft_list_push_back(&s2, &y);
			ft_list_push_back(&s2, &z);
			ft_sorted_list_merge(&s, s2, cmp_int_ptr);
			check(ft_list_size(s) == 7 && *(int *)s->data == 5
				&& *(int *)s->next->data == 10
				&& *(int *)s->next->next->data == 20
				&& *(int *)s->next->next->next->data == 22
				&& *(int *)s->next->next->next->next->data == 25
				&& *(int *)s->next->next->next->next->next->data == 30
				&& *(int *)ft_list_last(s)->data == 40,
				"ex17 sorted_list_merge");
			ft_list_clear(s, NULL);
		}
	}
	{
		t_list	*s = 0;

		ft_list_reverse(&s);
		check(s == NULL, "ex08 reverse liste vide");
		ft_list_reverse_fun(s);
		check(ft_list_size(s) == 0, "ex15 reverse_fun liste vide");
		ft_list_clear(s, NULL);
		check(1, "ex06 clear liste vide");
	}
	printf("C12 : %d reussi(s), %d echec(s)\n", g_pass, g_fail);
	return (g_fail != 0);
}
