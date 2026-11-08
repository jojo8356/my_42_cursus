/* Structure d'arbre binaire et prototype de btree_create_node. */
#ifndef FT_BTREE_H
# define FT_BTREE_H

typedef struct s_btree
{
	struct s_btree	*left;
	struct s_btree	*right;
	void			*data;
}	t_btree;

t_btree	*btree_create_node(void *item);

#endif
