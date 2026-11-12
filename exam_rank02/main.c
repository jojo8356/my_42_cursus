/* ************************************************************************** */
/*                                                                          */
/*                                                       :::      ::::::::   */
/*   main.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 21:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 21:00:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "exam_rank02.h"
# include <unistd.h>
# include <stdlib.h>
# include <string.h>

int main(void)
{
    /* Demo of some exercises */
    char str[] = "Hello";
    char *dup = ft_strdup(str);
    ft_putstr(dup);
    ft_putstr("\n");

    char rev[] = "hello";
    ft_strrev(rev);
    ft_putstr(rev);
    ft_putstr("\n");

    char *sorted = ulstr(strdup("Hello"));
    ft_putstr(sorted);
    ft_putstr("\n");
    free(sorted);

    char *num = ft_itoa(42);
    ft_putstr(num);
    ft_putstr("\n");
    free(num);

    return (0);
}
