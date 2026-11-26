/* ************************************************************************** */
/* ** */
/* ** */
/*   heap.h */
/* ** */
/*   By: jojo8356 */
/* ** */
/*   Created: 8 Nov by jojo8356 */
/*   Updated: 8 Nov by jojo8356 */
/* ** */
/* ************************************************************************** */
#ifndef HEAP_H
# define HEAP_H

# include "codexion.h"

int			waiter_less(t_waiter *a, t_waiter *b);
void		heap_sift_up(t_dongle *d, int idx);
void		heap_sift_down(t_dongle *d, int idx);
void		heap_push(t_dongle *d, t_waiter *w);
t_waiter	*heap_pop(t_dongle *d);
void		heap_remove(t_dongle *d, t_waiter *w);
#endif
