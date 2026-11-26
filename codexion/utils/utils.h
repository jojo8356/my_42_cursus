/* ************************************************************************** */
/* ** */
/* ** */
/*   utils.h */
/* ** */
/*   By: jojo8356 */
/* ** */
/*   Created: 8 Nov by jojo8356 */
/*   Updated: 8 Nov by jojo8356 */
/* ** */
/* ************************************************************************** */
#ifndef UTILS_H
# define UTILS_H

# include "codexion.h"

long long	now_ms(void);
long long	rel_now_ms(t_sim *sim);
void		sim_sleep(t_sim *sim, long ms);
int			parse_strict_int(const char *str, long *out);
int			stop_is_set(t_sim *sim);
#endif
