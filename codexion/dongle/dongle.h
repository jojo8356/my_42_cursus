/* ************************************************************************** */
/* ** */
/* ** */
/*   dongle.h */
/* ** */
/*   By: jojo8356 */
/* ** */
/*   Created: 8 Nov by jojo8356 */
/*   Updated: 8 Nov by jojo8356 */
/* ** */
/* ************************************************************************** */
#ifndef DONGLE_H
# define DONGLE_H

# include "codexion.h"

void		dongle_init(t_dongle *d, int cap);
t_waiter	*dongle_acquire(t_dongle *d, t_coder *c, t_sim *sim);
void		dongle_release(t_dongle *d, t_waiter *w, t_sim *sim);
#endif
