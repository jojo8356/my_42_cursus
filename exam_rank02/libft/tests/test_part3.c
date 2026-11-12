#include "test_framework.h"

static int	g_del_calls;

static void	counted_del(void *content)
{
	g_del_calls++;
	free(content);
}

static void	triple_int(void *content)
{
	if (content)
		*(int *)content *= 3;
}

static void	*dup_plus_ten(void *content)
{
	int	*p = (int *)malloc(sizeof(int));

	if (!p)
		return (NULL);
	*p = *(int *)content + 10;
	return (p);
}

static int	g_map_calls;

static void	*fail_on_third_node(void *content)
{
	int	*p;

	g_map_calls++;
	if (g_map_calls == 3)
		return (NULL);
	p = (int *)malloc(sizeof(int));
	if (!p)
		return (NULL);
	*p = *(int *)content * 2;
	return (p);
}

static int	*box_int(int v)
{
	int	*p = (int *)malloc(sizeof(int));

	if (p)
		*p = v;
	return (p);
}

static void	test_ft_lstnew(void)
{
	int		val = 42;
	t_list	*n1;
	t_list	*n2;

	BEGIN_FN_TEST("ft_lstnew");
	n1 = ft_lstnew(&val);
	T_ASSERT(n1 != NULL, "allocates node");
	T_ASSERT(n1->content == &val && *(int *)n1->content == 42, "stores exact content pointer");
	T_ASSERT(n1->next == NULL, "next initialized to NULL");
	T_ASSERT(exact_alloc_matches(n1, sizeof(t_list)), "allocates exact sizeof(t_list)");
	free(n1);

	n2 = ft_lstnew(NULL);
	T_ASSERT(n2 != NULL && n2->content == NULL && n2->next == NULL, "ft_lstnew(NULL) valid");
	free(n2);
	END_FN_TEST("ft_lstnew");
}

static void	test_ft_lstadd_front(void)
{
	t_list	*head = NULL;
	t_list	*a = ft_lstnew("A");
	t_list	*b = ft_lstnew("B");
	t_list	*c = ft_lstnew("C");

	BEGIN_FN_TEST("ft_lstadd_front");
	ft_lstadd_front(NULL, a);
	ft_lstadd_front(&head, NULL);
	T_ASSERT(head == NULL, "NULL guards leave head NULL");

	ft_lstadd_front(&head, a);
	T_ASSERT(head == a && head->next == NULL, "insert into empty list");
	ft_lstadd_front(&head, b);
	T_ASSERT(head == b && head->next == a && a->next == NULL, "insert 2nd at front");
	ft_lstadd_front(&head, c);
	T_ASSERT(head == c && head->next == b && b->next == a, "order C -> B -> A");
	free(a);
	free(b);
	free(c);
	END_FN_TEST("ft_lstadd_front");
}

static void	test_ft_lstsize(void)
{
	t_list	*head = NULL;

	BEGIN_FN_TEST("ft_lstsize");
	/* Subject v19.3 exact return type check: MUST be unsigned int (not int) */
	T_ASSERT(_Generic((ft_lstsize(NULL)), unsigned int: 1, default: 0) == 1,
		"return type is strictly 'unsigned int' per Subject v19.3");
	T_ASSERT(ft_lstsize(NULL) == 0U, "empty list -> 0");

	for (unsigned int i = 1; i <= 50; i++)
	{
		ft_lstadd_front(&head, ft_lstnew(NULL));
		T_ASSERT(ft_lstsize(head) == i, "size increments accurately up to 50");
	}
	ft_lstclear(&head, NULL);
	END_FN_TEST("ft_lstsize");
}

static void	test_ft_lstlast(void)
{
	t_list	*head = NULL;
	t_list	*first = ft_lstnew("1");
	t_list	*mid = ft_lstnew("2");
	t_list	*last = ft_lstnew("3");

	BEGIN_FN_TEST("ft_lstlast");
	T_ASSERT(ft_lstlast(NULL) == NULL, "empty list -> NULL");
	head = first;
	T_ASSERT(ft_lstlast(head) == first, "1-element list");
	first->next = mid;
	T_ASSERT(ft_lstlast(head) == mid, "2-element list");
	mid->next = last;
	T_ASSERT(ft_lstlast(head) == last, "3-element list");
	free(first);
	free(mid);
	free(last);
	END_FN_TEST("ft_lstlast");
}

static void	test_ft_lstadd_back(void)
{
	t_list	*head = NULL;
	t_list	*n1 = ft_lstnew("1");
	t_list	*n2 = ft_lstnew("2");
	t_list	*n3 = ft_lstnew("3");

	BEGIN_FN_TEST("ft_lstadd_back");
	ft_lstadd_back(NULL, n1);
	ft_lstadd_back(&head, NULL);
	T_ASSERT(head == NULL, "NULL guards");

	ft_lstadd_back(&head, n1);
	T_ASSERT(head == n1 && n1->next == NULL, "add_back on empty list updates head");
	ft_lstadd_back(&head, n2);
	ft_lstadd_back(&head, n3);
	T_ASSERT(head == n1 && n1->next == n2 && n2->next == n3 && n3->next == NULL,
		"preserves FIFO order 1 -> 2 -> 3");
	free(n1);
	free(n2);
	free(n3);
	END_FN_TEST("ft_lstadd_back");
}

static void	test_ft_lstdelone(void)
{
	t_list	*n1 = ft_lstnew(box_int(111));
	t_list	*n2 = ft_lstnew(box_int(222));

	BEGIN_FN_TEST("ft_lstdelone");
	n1->next = n2;
	g_del_calls = 0;
	ft_lstdelone(NULL, counted_del);
	T_ASSERT(g_del_calls == 0, "NULL node safe");

	ft_lstdelone(n1, counted_del);
	T_ASSERT(g_del_calls == 1, "del called once on n1->content");
	T_ASSERT(*(int *)n2->content == 222, "n1->next (n2) was NOT freed or corrupted");
	ft_lstdelone(n2, counted_del);
	T_ASSERT(g_del_calls == 2, "del called on n2->content");
	END_FN_TEST("ft_lstdelone");
}

static void	test_ft_lstclear(void)
{
	t_list	*head = NULL;

	BEGIN_FN_TEST("ft_lstclear");
	ft_lstclear(NULL, counted_del);
	ft_lstclear(&head, counted_del);
	T_ASSERT(head == NULL, "clearing empty list safe");

	for (int i = 0; i < 10; i++)
		ft_lstadd_back(&head, ft_lstnew(box_int(i)));
	g_del_calls = 0;
	ft_lstclear(&head, counted_del);
	T_ASSERT(g_del_calls == 10, "del called on all 10 nodes");
	T_ASSERT(head == NULL, "head pointer set to NULL");
	END_FN_TEST("ft_lstclear");
}

static void	test_ft_lstiter(void)
{
	t_list	*head = NULL;
	int		vals[4] = {1, 2, -5, 14};

	BEGIN_FN_TEST("ft_lstiter");
	ft_lstiter(NULL, triple_int);
	for (int i = 0; i < 4; i++)
		ft_lstadd_back(&head, ft_lstnew(&vals[i]));
	ft_lstiter(head, NULL);
	T_ASSERT(vals[0] == 1 && vals[3] == 14, "f=NULL leaves list untouched");

	ft_lstiter(head, triple_int);
	T_ASSERT(vals[0] == 3 && vals[1] == 6 && vals[2] == -15 && vals[3] == 42,
		"applies f to every node in order");
	ft_lstclear(&head, NULL);
	END_FN_TEST("ft_lstiter");
}

static void	test_ft_lstmap(void)
{
	t_list	*orig = NULL;
	t_list	*mapped;
	t_list	*null_mapped;
	int		vals[4] = {5, 10, 15, 32};

	BEGIN_FN_TEST("ft_lstmap");
	T_ASSERT(ft_lstmap(NULL, dup_plus_ten, counted_del) == NULL, "lst=NULL -> NULL");
	for (int i = 0; i < 4; i++)
		ft_lstadd_back(&orig, ft_lstnew(&vals[i]));
	T_ASSERT(ft_lstmap(orig, NULL, counted_del) == NULL, "f=NULL -> NULL");

	mapped = ft_lstmap(orig, dup_plus_ten, counted_del);
	T_ASSERT(mapped != NULL && mapped != orig, "creates distinct list");
	T_ASSERT(ft_lstsize(mapped) == 4U, "mapped list has 4 nodes");
	T_ASSERT(*(int *)mapped->content == 15
		&& *(int *)mapped->next->content == 20
		&& *(int *)mapped->next->next->content == 25
		&& *(int *)mapped->next->next->next->content == 42,
		"preserves exact node order and transformed contents");
	T_ASSERT(vals[0] == 5 && vals[1] == 10 && vals[2] == 15 && vals[3] == 32,
		"original list untouched");

	/* f failing partway through the list (node 3 of 4): frees partial list via del and returns NULL */
	g_map_calls = 0;
	g_del_calls = 0;
	null_mapped = ft_lstmap(orig, fail_on_third_node, counted_del);
	T_ASSERT(null_mapped == NULL && g_del_calls == 2,
		"f failing on 3rd node frees first 2 nodes via del and returns NULL");
	T_ASSERT(ft_lstsize(orig) == 4U && vals[0] == 5 && vals[3] == 32,
		"original list intact after mid-list f failure");

	ft_lstclear(&mapped, counted_del);
	ft_lstclear(&orig, NULL);
	END_FN_TEST("ft_lstmap");
}

void	run_part3_tests(void)
{
	printf(CLR_BOLD CLR_CYAN "\n--- Partie 3 : Listes chainees t_list (9 fonctions — Obligatoire v19.3) ---" CLR_RESET "\n");
	test_ft_lstnew();
	test_ft_lstadd_front();
	test_ft_lstsize();
	test_ft_lstlast();
	test_ft_lstadd_back();
	test_ft_lstdelone();
	test_ft_lstclear();
	test_ft_lstiter();
	test_ft_lstmap();
}
