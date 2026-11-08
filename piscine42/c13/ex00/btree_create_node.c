/* Cree un noeud d'arbre contenant item, fils gauche et droit a 0. */
#include <stdlib.h>
#include "ft_btree.h"

t_btree	*btree_create_node(void *item)
{
	t_btree	*node;

	node = malloc(sizeof(t_btree));
	if (!node)
		return (0);
	node->data = item;
	node->left = 0;
	node->right = 0;
	return (node);
}
