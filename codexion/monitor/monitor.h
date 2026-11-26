/* ************************************************************************** */
/* ** */
/* ** */
/*   monitor.h */
/* ** */
/*   By: jojo8356 */
/* ** */
/*   Created: 8 Nov by jojo8356 */
/*   Updated: 8 Nov by jojo8356 */
/* ** */
/* ************************************************************************** */
#ifndef MONITOR_H
# define MONITOR_H

# include "codexion.h"

void	set_stop(t_sim *sim);
int		all_done(t_sim *sim);
void	coder_burned(t_sim *sim);
void	*monitor_routine(void *arg);
#endif
