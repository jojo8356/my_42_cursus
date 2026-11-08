/* Tests complets du jour C13 (arbres binaires de recherche). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../c13/ft_btree.h"

void	btree_apply_prefix(t_btree *root, void (*applyf)(void *));
void	btree_apply_infix(t_btree *root, void (*applyf)(void *));
void	btree_apply_suffix(t_btree *root, void (*applyf)(void *));
void	btree_insert_data(t_btree **root, void *item,
			int (*cmpf)(void *, void *));
void	*btree_search_item(t_btree *root, void *data_ref,
			int (*cmpf)(void *, void *));
int		btree_level_count(t_btree *root);
void	btree_apply_by_level(t_btree *root,
			void (*applyf)(void *item, int level, int is_first));

static int	g_pass = 0;
static int	g_fail = 0;
static char	g_buf[256];
static int	g_pos = 0;

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

static void	append(void *data)
{
	int	i;

	i = 0;
	while (((char *)data)[i])
	{
		g_buf[g_pos++] = ((char *)data)[i];
		i++;
	}
	g_buf[g_pos++] = '-';
	g_buf[g_pos] = 0;
}

static void	append_level(void *data, int level, int is_first)
{
	char	nb[2];

	nb[0] = (char)('0' + level);
	nb[1] = 0;
	if (is_first)
	{
		g_buf[g_pos++] = '[';
	}
	append(data);
	g_buf[g_pos++] = nb[0];
	g_buf[g_pos++] = ',';
	g_buf[g_pos] = 0;
}

static void	reset_buf(void)
{
	g_pos = 0;
	g_buf[0] = 0;
}

static void	free_data(void *data)
{
	free(data);
}

static void	free_tree(t_btree *root)
{
	if (!root)
		return ;
	free_tree(root->left);
	free_tree(root->right);
	free(root);
}

int	main(void)
{
	t_btree	*root;
	t_btree	*node;
	char	*data[7] = {"f", "c", "i", "a", "e", "h", "j"};
	int		i;

	printf("== C13 ==\n");
	node = btree_create_node("x");
	check(node != NULL && node->data == (void *)"x"
		&& node->left == NULL && node->right == NULL, "ex00 create_node");
	free(node);
	root = NULL;
	i = 0;
	while (i < 7)
	{
		btree_insert_data(&root, data[i], cmp_str);
		i++;
	}
	check(root != NULL && strcmp((char *)root->data, "f") == 0,
		"ex04 racine = 'f'");
	check(root->left != NULL && strcmp((char *)root->left->data, "c") == 0,
		"ex04 gauche = 'c'");
	check(root->right != NULL && strcmp((char *)root->right->data, "i") == 0,
		"ex04 droite = 'i'");
	reset_buf();
	btree_apply_prefix(root, append);
	check(strcmp(g_buf, "f-c-a-e-i-h-j-") == 0, "ex01 prefixe f c a e i h j");
	reset_buf();
	btree_apply_infix(root, append);
	check(strcmp(g_buf, "a-c-e-f-h-i-j-") == 0, "ex02 infixe a c e f h i j");
	reset_buf();
	btree_apply_suffix(root, append);
	check(strcmp(g_buf, "a-e-c-h-j-i-f-") == 0, "ex03 suffixe a e c h j i f");
	check(strcmp((char *)btree_search_item(root, (void *)"e", cmp_str),
			"e") == 0, "ex05 search 'e'");
	check(btree_search_item(root, (void *)"z", cmp_str) == NULL,
		"ex05 search absent -> NULL");
	check(btree_search_item(NULL, (void *)"e", cmp_str) == NULL,
		"ex05 search arbre vide");
	check(btree_level_count(root) == 3, "ex06 level_count == 3");
	check(btree_level_count(NULL) == 0, "ex06 level_count vide == 0");
	reset_buf();
	btree_apply_by_level(root, append_level);
	check(strcmp(g_buf, "[f-0,[c-1,i-1,[a-2,e-2,h-2,j-2,") == 0,
		"ex07 apply_by_level f / c i / a e h j");
	{
		t_btree	*line = NULL;

		btree_insert_data(&line, (void *)"a", cmp_str);
		btree_insert_data(&line, (void *)"b", cmp_str);
		reset_buf();
		btree_apply_by_level(line, append_level);
		check(strcmp(g_buf, "[a-0,[b-1,") == 0, "ex07 arbre en chaine");
		check(btree_level_count(line) == 2, "ex06 chaine de 2 niveaux");
		free_tree(line);
	}
	free_tree(root);
	{
		t_btree	*heap = NULL;
		char	*dup[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
		char	*src[7] = {"f", "c", "i", "a", "e", "h", "j"};

		i = 0;
		while (i < 7)
		{
			dup[i] = strdup(src[i]);
			btree_insert_data(&heap, dup[i], cmp_str);
			i++;
		}
		i = 0;
		while (i < 7)
		{
			btree_insert_data(&heap, strdup("zz"), cmp_str);
			i++;
		}
		free_tree(heap);
		check(1, "liberation arbre double (valgrind)");
		free_data(NULL);
	}
	printf("C13 : %d reussi(s), %d echec(s)\n", g_pass, g_fail);
	return (g_fail != 0);
}
