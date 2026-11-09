/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   charset_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 22:26:08 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/08 22:26:08 by jojo8356         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/*
** Bonus build: 'X' is the enemy marker, and the only symbol the bonus adds to
** the five of chapter IV.
*/

int	bonus_charset(char c)
{
	return (c == T_ENEMY);
}
