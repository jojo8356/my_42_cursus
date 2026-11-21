/* ************************************************************************** */
/*                                                                            */
/*   routing.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Routage : plus long préfixe correspondant.                          */
/* ************************************************************************** */

# include "netpratice.h"

/* Retourne l'index de la route dont le préfixe correspond à `ip`,
   en privilégiant le préfixe le plus long. -1 si aucune. */
int             route_match(const t_route *routes, int n, t_ip ip)
{
    int            best = -1;
    int            best_cidr = -1;
    int            i;
    int            cidr;

    for (i = 0; i < n; i++)
    {
        cidr = mask_to_cidr(routes[i].dest_mask);
        if (cidr < 0)
            continue;
        if (ip_apply_mask(ip, routes[i].dest_mask) == routes[i].dest)
        {
            if (cidr > best_cidr)
            {
                best_cidr = cidr;
                best = i;
            }
        }
    }
    return (best);
}
