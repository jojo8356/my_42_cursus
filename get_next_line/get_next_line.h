/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:30:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:30:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

# include <stdlib.h>
# include <unistd.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 42
# endif

typedef struct s_stash
{
	char	*buf;
	size_t	len;
	size_t	cap;
	size_t	scan;
}	t_stash;

char	*get_next_line(int fd);
void	gnl_clear(t_stash *s);
void	gnl_memcpy(char *dst, const char *src, size_t n);
int		gnl_find_nl(t_stash *s, size_t *nl_idx);
int		gnl_reserve(t_stash *s, size_t add);

#endif
