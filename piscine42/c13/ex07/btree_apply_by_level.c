/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   btree_apply_by_level.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Applique applyf a chaque noeud niveau par niveau (gauche -> droite).
   applyf(data, niveau, est_premier_du_niveau) avec niveau 0 = racine. */
#include "ft_btree.h"

static int	tree_height(t_btree *root)
{
	int	left;
	int	right;

	if (!root)
		return (0);
	left = tree_height(root->left);
	right = tree_height(root->right);
	if (left > right)
		return (left + 1);
	return (right + 1);
}

static void	apply_at_level(t_btree *root, int depth, int level,
		int *first, void (*applyf)(void *, int, int))
{
	if (!root)
		return ;
	if (depth == 0)
	{
		applyf(root->data, level, *first);
		*first = 0;
		return ;
	}
	apply_at_level(root->left, depth - 1, level, first, applyf);
	apply_at_level(root->right, depth - 1, level, first, applyf);
}

void	btree_apply_by_level(t_btree *root,
		void (*applyf)(void *item, int current_level, int is_first_elem))
{
	int	level;
	int	first;
	int	height;

	height = tree_height(root);
	level = 0;
	while (level < height)
	{
		first = 1;
		apply_at_level(root, level, level, &first, applyf);
		level++;
	}
}
