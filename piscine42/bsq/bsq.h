/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bsq.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* BSQ : plus grand carre d'emplacements libres sur une carte. */
#ifndef BSQ_H
# define BSQ_H

typedef struct s_map
{
	int		rows;
	int		cols;
	char	empty;
	char	obst;
	char	full;
	char	**grid;
}	t_map;

char	*read_fd(int fd, int *len);
int		load_map(char *buf, int len, t_map *m);
void	free_map(t_map *m);
void	solve(t_map *m);
void	ft_putstr(char *s);

#endif
