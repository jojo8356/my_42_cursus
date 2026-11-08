/* Parcours prefixe : racine, gauche, droite. */
#include "ft_btree.h"

void	btree_apply_prefix(t_btree *root, void (*applyf)(void *))
{
	if (!root)
		return ;
	applyf(root->data);
	btree_apply_prefix(root->left, applyf);
	btree_apply_prefix(root->right, applyf);
}
