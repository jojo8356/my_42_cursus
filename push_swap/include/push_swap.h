/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   push_swap.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 21:36:54 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 21:36:54 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PUSH_SWAP_H
# define PUSH_SWAP_H

# include <stdlib.h>
# include <unistd.h>
# include "../libft/libft.h"

/*
** Everything below is sized so that 500 values stay fast: the algorithm is
** O(n^2) in the worst case (which is fine for n <= 500) but the constant is
** small because the stacks are contiguous arrays, not linked lists.
*/

typedef struct s_stack
{
	int		*data;
	int		size;
	int		capacity;
}	t_stack;

typedef struct s_ops
{
	char	*buf;
	int		len;
	int		cap;
	int		count;
}	t_ops;

/* ---- parsing ---- */
int		ps_parse(int ac, char **av, t_stack *a);
int		ps_str_to_int(const char *s, int *out);
int		ps_is_blank(char c);
int		ps_fill(t_stack *a, char *s, int *k);
int		ps_has_duplicate(const t_stack *a);
void	ps_error(void);

/* ---- stack primitives (no output) ---- */
int		st_init(t_stack *s, int capacity);
void	st_free(t_stack *s);
void	st_swap(t_stack *s);
void	st_rotate(t_stack *s);
void	st_reverse(t_stack *s);
int		st_push(t_stack *dst, t_stack *src);
void	st_copy(t_stack *dst, const t_stack *src);

/* ---- recorded instructions (the program's output) ---- */
int		op_emit(t_ops *o, const char *name);
int		op_flush(t_ops *o);
void	op_free(t_ops *o);

/* ---- instructions, both recorded and applied ---- */
void	do_sa(t_stack *a, t_stack *b, t_ops *o);
void	do_sb(t_stack *a, t_stack *b, t_ops *o);
void	do_ss(t_stack *a, t_stack *b, t_ops *o);
void	do_pa(t_stack *a, t_stack *b, t_ops *o);
void	do_pb(t_stack *a, t_stack *b, t_ops *o);
void	do_ra(t_stack *a, t_stack *b, t_ops *o);
void	do_rb(t_stack *a, t_stack *b, t_ops *o);
void	do_rr(t_stack *a, t_stack *b, t_ops *o);
void	do_rra(t_stack *a, t_stack *b, t_ops *o);
void	do_rrb(t_stack *a, t_stack *b, t_ops *o);
void	do_rrr(t_stack *a, t_stack *b, t_ops *o);

/* ---- dispatch by name (shared with the checker) ---- */
int		exec_instr(t_stack *a, t_stack *b, const char *name, t_ops *o);

/* ---- algorithms ---- */
int		is_sorted(const t_stack *a);
void	sort_two(t_stack *a, t_stack *b, t_ops *o);
void	sort_three(t_stack *a, t_stack *b, t_ops *o);
void	sort_small(t_stack *a, t_stack *b, t_ops *o);
void	sort_chunk(t_stack *a, t_stack *b, t_ops *o);
void	sort_adaptive(t_stack *a, t_stack *b, t_ops *o);
void	sort_insert(t_stack *a, t_stack *b, t_ops *o);
int		ps_band_width(int n);
int		rot_cost(int size, int idx);
int		pair_cost(int sa, int ia, int sb, int ib);
void	rotate_pair(t_stack *a, t_stack *b, int ia, int ib, t_ops *o);
void	ps_rotate_to(t_stack *a, t_stack *b, int idx, int which, t_ops *o);
int		ps_b_place(const t_stack *b, int v, int *pos);

/* ---- helpers ---- */
int		ps_rank(t_stack *a);
int		index_of_min(const t_stack *a);
int		index_of_max(const t_stack *a);
int		value_at(const t_stack *s, int i);
int		ps_strlen(const char *s);
int		op_init(t_ops *o, int cap);

#endif
