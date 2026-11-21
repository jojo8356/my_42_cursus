/* ************************************************************************** */
/*                                                                            */
/*   validate.c                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Validation config + simulation de routage (BFS / plus long préfixe).*/
/* ************************************************************************** */

# include "netpratice.h"

/* ---- helpers : accès par nom ---- */

static t_iface    *find_iface(const t_solution *sol, const char *name)
{
    int            i;

    for (i = 0; i < sol->n_ifs; i++)
    {
        if (ft_strcmp(sol->ifs[i].name, name) == 0)
            return (&sol->ifs[i]);
    }
    return (NULL);
}

static t_rt_table    *find_rt(const t_solution *sol, const char *name)
{
    int            i;

    for (i = 0; i < sol->n_rts; i++)
    {
        if (ft_strcmp(sol->rts[i].name, name) == 0)
            return (&sol->rts[i]);
    }
    return (NULL);
}

/* Vérifie qu'une adresse n'est ni réseau ni broadcast de son sous-réseau. */
static int        is_network_or_broadcast(t_ip ip, uint32_t mask)
{
    t_subnet        s = subnet_new(ip, mask);

    return (ip == s.network || ip == subnet_broadcast(&s));
}

/* La passerelle `ip` est-elle sur un sous-réseau directement connecté
   du device qui possède la table de routage `rtname` ? */
static int        gateway_on_connected_subnet(const t_level *lvl,
                const t_solution *sol, const char *rtname, t_ip gw)
{
    int            d;
    int            k;

    for (d = 0; d < lvl->n_devices; d++)
    {
        const char        *rtn = lvl->devices[d].rt_name
            ? lvl->devices[d].rt_name : lvl->devices[d].name;
        if (rtn && ft_strcmp(rtn, rtname) != 0)
            continue;
        for (k = 0; k < lvl->devices[d].n_ifs; k++)
        {
            t_iface        *ifc = find_iface(sol, lvl->devices[d].if_names[k]);
            if (ifc && ifc->has_ip && ifc->has_mask)
            {
                t_subnet        s = subnet_new(ifc->ip, ifc->mask);
                if (subnet_contains(&s, gw))
                    return (1);
            }
        }
    }
    return (0);
}

int            solution_validate_config(const t_solution *sol,
                const t_level *lvl, char **err)
{
    int            i;
    int            j;

    (void)lvl;
    if (!sol)
    {
        *err = ft_strdup("solution vide");
        return (-1);
    }
    /* masques + IP valides */
    for (i = 0; i < sol->n_ifs; i++)
    {
        t_iface        *ifc = &sol->ifs[i];

        if (ifc->has_ip && !ip_is_valid(ifc->ip))
        {
            *err = ft_strdup("IP invalide sur une interface");
            return (-1);
        }
        if (ifc->has_mask && !mask_is_valid(ifc->mask))
        {
            *err = ft_strdup("masque non contigu invalide");
            return (-1);
        }
        if (ifc->has_ip && ifc->has_mask
            && is_network_or_broadcast(ifc->ip, ifc->mask))
        {
            *err = ft_strdup("IP = adresse réseau ou broadcast");
            return (-1);
        }
    }
    /* passerelles : IP valide + sur un sous-réseau connecté (si niveau donné) */
    for (i = 0; i < sol->n_rts; i++)
    {
        t_rt_table        *rt = &sol->rts[i];

        for (j = 0; j < rt->count; j++)
        {
            t_rt_entry        *e = &rt->entries[j];

            if (!e->has_gate)
                continue;
            if (!ip_is_valid(e->gateway))
            {
                *err = ft_strdup("passerelle invalide");
                return (-1);
            }
            if (lvl && !gateway_on_connected_subnet(lvl, sol, rt->name,
                    e->gateway))
            {
                *err = ft_strdup("passerelle non sur un sous-réseau connecté");
                return (-1);
            }
        }
    }
    return (0);
}

/* ---- simulation de routage ---- */

typedef struct s_dev_ctx
{
    const t_solution        *sol;
    const t_level            *lvl;
    char            *visited;    /* masque visite par index de device */
    int            n_dev;
}                   t_dev_ctx;

static int        device_index(const t_level *lvl, const char *name)
{
    int            i;

    for (i = 0; i < lvl->n_devices; i++)
    {
        if (ft_strcmp(lvl->devices[i].name, name) == 0)
            return (i);
    }
    return (-1);
}

/* Retourne le device propriétaire de l'IP `ip` (premier candidat). */
static int        owner_of_ip(const t_level *lvl, const t_solution *sol,
                t_ip ip)
{
    int            d;
    int            k;

    for (d = 0; d < lvl->n_devices; d++)
    {
        for (k = 0; k < lvl->devices[d].n_ifs; k++)
        {
            const char            *iname = lvl->devices[d].if_names[k];
            t_iface            *ifc = find_iface(sol, iname);

            if (ifc && ifc->has_ip && ifc->ip == ip)
                return (d);
        }
    }
    return (-1);
}

/* Le device `didx` peut-il atteindre `dest_ip` ? (récursif avec visite) */
static int        device_reach(const t_level *lvl, const t_solution *sol,
                int didx, t_ip dest_ip, char *visited)
{
    const t_device_def        *dev = &lvl->devices[didx];
    t_iface        *ifs[16];
    int            n_ifs = 0;
    int            k;
    int            i;

    /* interfaces directement connectées */
    for (k = 0; k < dev->n_ifs && n_ifs < 16; k++)
    {
        t_iface            *ifc = find_iface(sol, dev->if_names[k]);
        if (ifc && ifc->has_ip && ifc->has_mask)
            ifs[n_ifs++] = ifc;
    }
    /* direct : dest dans un sous-réseau connecté */
    for (i = 0; i < n_ifs; i++)
    {
        t_subnet        s = subnet_new(ifs[i]->ip, ifs[i]->mask);
        if (subnet_contains(&s, dest_ip))
            return (1);
    }
    /* routage : plus long préfixe */
    {
        const char        *rtname = dev->rt_name ? dev->rt_name : dev->name;
        t_rt_table        *rt = find_rt(sol, rtname);
        t_route            routes[64];
        int            n_routes = 0;

        if (rt)
        {
            for (k = 0; k < rt->count && n_routes < 64; k++)
            {
                t_rt_entry        *e = &rt->entries[k];
                if (e->has_route)
                {
                    routes[n_routes].dest = e->dest;
                    routes[n_routes].dest_mask = e->dest_mask;
                    routes[n_routes].gateway = e->gateway;
                    n_routes++;
                }
            }
        }
        {
            int            best = route_match(routes, n_routes, dest_ip);
            if (best < 0)
                return (0);
            if (!routes[best].gateway)
                return (0);
            {
                int            nidx = owner_of_ip(lvl, sol, routes[best].gateway);
                if (nidx < 0)
                    return (0);
                if (visited[nidx])
                    return (0);
                visited[nidx] = 1;
                if (device_reach(lvl, sol, nidx, dest_ip, visited))
                {
                    visited[nidx] = 0;
                    return (1);
                }
                visited[nidx] = 0;
            }
        }
    }
    return (0);
}

/* IP principale d'un device (première interface avec IP) */
static t_ip        device_main_ip(const t_level *lvl, const t_solution *sol,
                const char *dev_name)
{
    int            didx = device_index(lvl, dev_name);
    int            k;

    if (didx < 0)
        return (0);
    for (k = 0; k < lvl->devices[didx].n_ifs; k++)
    {
        t_iface        *ifc = find_iface(sol, lvl->devices[didx].if_names[k]);
        if (ifc && ifc->has_ip)
            return (ifc->ip);
    }
    return (0);
}

int            can_reach(const t_solution *sol, const t_level *lvl,
                const char *src_dev, const char *dst_dev)
{
    t_ip            src_ip = device_main_ip(lvl, sol, src_dev);
    t_ip            dst_ip = device_main_ip(lvl, sol, dst_dev);
    char            visited[64];
    int            didx;

    if (!src_ip || !dst_ip)
        return (0);
    didx = device_index(lvl, src_dev);
    if (didx < 0)
        return (0);
    ft_bzero(visited, sizeof(visited));
    visited[didx] = 1;
    return (device_reach(lvl, sol, didx, dst_ip, visited));
}

int            solution_validate_goals(const t_solution *sol,
                const t_level *lvl, char **err)
{
    int            i;

    if (solution_validate_config(sol, lvl, err) != 0)
        return (-1);
    for (i = 0; i < lvl->n_goals; i++)
    {
        if (!can_reach(sol, lvl, lvl->goals[i].src, lvl->goals[i].dst))
        {
            char            tmp[128];
            ft_snprintf(tmp, sizeof(tmp), "objectif %s -> %s non atteint",
                lvl->goals[i].src, lvl->goals[i].dst);
            *err = ft_strdup(tmp);
            return (-1);
        }
    }
    return (0);
}
