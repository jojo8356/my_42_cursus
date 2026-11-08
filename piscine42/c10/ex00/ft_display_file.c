/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_display_file.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 19:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Affiche le contenu du fichier passe en argument (un seul argument).
   Pas de malloc : lecture par blocs dans un tableau de taille fixe. */
#include <unistd.h>
#include <fcntl.h>

static void	put_msg(int fd, char *msg)
{
	int	len;

	len = 0;
	while (msg[len])
		len++;
	write(fd, msg, len);
	write(fd, "\n", 1);
}

int	main(int argc, char **argv)
{
	char	buf[29696];
	int		fd;
	int		n;

	if (argc < 2)
		return (put_msg(2, "File name missing."), 1);
	if (argc > 2)
		return (put_msg(2, "Too many arguments."), 1);
	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		return (put_msg(2, "Cannot read file."), 1);
	n = read(fd, buf, sizeof(buf));
	while (n > 0)
	{
		write(1, buf, n);
		n = read(fd, buf, sizeof(buf));
	}
	close(fd);
	return (0);
}
