/* ************************************************************************** */
/*                                                                            */
/*   solver.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Solveur : allocation de sous-réseaux + tables de routage (BFS).      */
/* ************************************************************************** */

# include "netpratice.h"

/* base d'allocation : 203.0.113.0/24 (TEST-NET-3, non privé) */
# define BASE_A 203
# define BASE_B 0
# define BASE_C 113

/* infos sur un sous-réseau du solveur */
typedef struct s_sinfo
{
    t_ip            network;
    uint32_t        mask;
    char            **ifaces;        /* noms d'interfaces             */
    int            *dev_idx;        /* index device de chaque iface */
    t_ip            *ips;            /* IP attribuée par interface    */
    int            n;
}                   t_sinfo;

/* cherche l'index du device possédant l'interface `iname` */
static int        device_of_iface(const t_level *lvl, const char *iname)
{
    int            d;
    int            k;

    for (d = 0; d < lvl->n_devices; d++)
    {
        for (k = 0; k < lvl->devices[d].n_ifs; k++)
        {
            if (ft_strcmp(lvl->devices[d].if_names[k], iname) == 0)
                return (d);
        }
    }
    return (-1);
}

static int        device_index_name(const t_level *lvl, const char *name)
{
    int            d;

    for (d = 0; d < lvl->n_devices; d++)
    {
        if (ft_strcmp(lvl->devices[d].name, name) == 0)
            return (d);
    }
    return (-1);
}

/* vrai si `s` appartient au tableau `arr` (de taille `n`). */
static int        subnet_of(const int *arr, int n, int s)
{
    int            i;

    for (i = 0; i < n; i++)
    {
        if (arr[i] == s)
            return (1);
    }
    return (0);
}

/* construit les infos de sous-réseaux + attribue IP et masque */
static int        build_subnets(const t_level *lvl, t_sinfo **out, int *n_out)
{
    t_sinfo        *subs;
    int            n = lvl->n_links;
    int            i;
    int            j;
    int            host_id;

    *out = NULL;
    *n_out = 0;
    if (n <= 0)
        return (0);
    subs = (t_sinfo *)xcalloc(n, sizeof(t_sinfo));
    for (i = 0; i < n; i++)
    {
        subs[i].n = lvl->links[i].n_ifs;
        subs[i].ifaces = (char **)xcalloc(subs[i].n, sizeof(char *));
        subs[i].dev_idx = (int *)xcalloc(subs[i].n, sizeof(int));
        subs[i].ips = (t_ip *)xcalloc(subs[i].n, sizeof(t_ip));
        for (j = 0; j < subs[i].n; j++)
            subs[i].ifaces[j] = ft_strdup(lvl->links[i].ifs[j]);
    }
    /* attribution réseau + IP : un /24 par sous-réseau (disjoint) */
    for (i = 0; i < n; i++)
    {
        subs[i].mask = cidr_to_mask(24);
        /* réseau : 203.0.(113+i).0/24 — chaque /24 est disjoint */
        subs[i].network = (t_ip)((BASE_A << 24) | (BASE_B << 16)
            | ((unsigned char)(BASE_C + i) << 8) | 0);
        /* IPs : premier hôte utilisable = network + 1 */
        host_id = 1;
        for (j = 0; j < subs[i].n; j++)
        {
            subs[i].dev_idx[j] = device_of_iface(lvl, subs[i].ifaces[j]);
            subs[i].ips[j] = subs[i].network + host_id;
            host_id++;
        }
    }
    *out = subs;
    *n_out = n;
    return (0);
}

/* pour un device, ses sous-réseaux directement connectés */
static int        dev_subnets(const t_level *lvl, const t_sinfo *subs,
                int n_subs, int dev, int *out, int *n_out)
{
    int            k;
    int            s;
    int            cnt = 0;

    (void)lvl;
    *n_out = 0;
    for (s = 0; s < n_subs; s++)
    {
        for (k = 0; k < subs[s].n; k++)
        {
            if (subs[s].dev_idx[k] == dev)
            {
                out[cnt++] = s;
                break;
            }
        }
    }
    *n_out = cnt;
    return (0);
}

/* build graphe : adjacency entre sous-réseaux via les routeurs */
static int        *build_adj(const t_level *lvl, const t_sinfo *subs,
                int n_subs, int *adj_size)
{
    /* adj[s] = bitmask des sous-réseaux adjacents */
    int            *adj = (int *)xcalloc(n_subs, sizeof(int));
    int            d;
    int            s1;
    int            s2;
    int            i1;
    int            i2;

    for (d = 0; d < lvl->n_devices; d++)
    {
        int            ds[16];
        int            nd = 0;
        int            a;
        int            b;

        dev_subnets(lvl, subs, n_subs, d, ds, &nd);
        for (a = 0; a < nd; a++)
        {
            for (b = a + 1; b < nd; b++)
            {
                s1 = ds[a];
                s2 = ds[b];
                adj[s1] |= (1 << s2);
                adj[s2] |= (1 << s1);
            }
        }
        (void)i1;
        (void)i2;
    }
    *adj_size = n_subs;
    return (adj);
}

/* BFS : plus court chemin entre un ensemble de sources et une cible.
   Retourne le tableau prev[] (prev[t] = sous-réseau précédent sur le
   chemin depuis la source), -1 si inaccessible. */
static void        bfs_from(const int *adj, int n, const int *sources,
                int n_src, int target, int *prev)
{
    int            visited[64];
    int            queue[64];
    int            qh = 0;
    int            qt = 0;
    int            i;
    int            cur;
    int            next;

    ft_bzero(visited, sizeof(visited));
    ft_bzero(prev, n * sizeof(int));
    for (i = 0; i < n; i++)
        prev[i] = -1;
    for (i = 0; i < n_src; i++)
    {
        if (sources[i] >= 0 && sources[i] < n)
        {
            visited[sources[i]] = 1;
            prev[sources[i]] = -2;    /* source */
            queue[qt++] = sources[i];
        }
    }
    while (qh < qt)
    {
        cur = queue[qh++];
        if (cur == target)
            break;
        for (next = 0; next < n; next++)
        {
            if ((adj[cur] & (1 << next)) && !visited[next])
            {
                visited[next] = 1;
                prev[next] = cur;
                queue[qt++] = next;
            }
        }
    }
}

/* next-hop router : pour le routeur `dev` (connecté à `src_sub`) atteindre
   `target_sub`, trouve le routeur voisin et son IP dans `src_sub`. */
static t_ip        next_hop_ip(const t_level *lvl, const t_sinfo *subs,
                int n_subs, int dev, int src_sub, int target_sub)
{
    (void)src_sub;
    int            ds[16];
    int            nd = 0;
    int            d;
    int            k;
    int            s;
    int            j;

    dev_subnets(lvl, subs, n_subs, dev, ds, &nd);
    /* BFS depuis les sous-réseaux de `dev` vers target_sub */
    {
        int            *adj = build_adj(lvl, subs, n_subs, &(int){0});
        int            prev[64];

        bfs_from(adj, n_subs, ds, nd, target_sub, prev);
        ft_free(adj);
        if (prev[target_sub] < 0)
            return (0);
        /* remonte le chemin jusqu'au premier sous-réseau directement
           connecté à dev (prev == -2) ; le saut suivant est le next hop. */
        {
            int            cur = target_sub;
            int            first_direct = -1;
            int            next_sub = -1;

            while (cur >= 0)
            {
                int            p = prev[cur];
                if (p == -2)
                {
                    first_direct = cur;
                    break;
                }
                cur = p;
            }
            if (first_direct < 0)
                return (0);
            /* le sous-réseau après first_direct sur le chemin vers target */
            cur = target_sub;
            while (prev[cur] != first_direct)
                cur = prev[cur];
            next_sub = cur;
            /* trouver un routeur (≠ dev) connectant first_direct et next_sub
               dont l'IP dans first_direct est la passerelle */
            for (d = 0; d < lvl->n_devices; d++)
            {
                if (d == dev)
                    continue;
                {
                    int            ds2[16];
                    int            nd2 = 0;
                    dev_subnets(lvl, subs, n_subs, d, ds2, &nd2);
                    for (s = 0; s < nd2; s++)
                    {
                        if (ds2[s] == first_direct)
                        {
                            for (k = 0; k < subs[first_direct].n; k++)
                            {
                                if (subs[first_direct].dev_idx[k] == d)
                                {
                                    (void)next_sub;
                                    (void)j;
                                    return (subs[first_direct].ips[k]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return (0);
}

int            solver_solve(const t_level *lvl, const t_topo *topo,
                t_solution *out)
{
    t_sinfo        *subs = NULL;
    int            n_subs = 0;
    int            *adj;
    int            adj_size = 0;
    int            d;
    int            s;
    int            i;
    int            k;

    (void)topo;
    ft_bzero(out, sizeof(*out));
    if (!lvl)
        return (-1);
    if (build_subnets(lvl, &subs, &n_subs) != 0)
        return (-1);
    adj = build_adj(lvl, subs, n_subs, &adj_size);
    (void)adj_size;

    /* ---- interfaces ---- */
    for (s = 0; s < n_subs; s++)
    {
        for (i = 0; i < subs[s].n; i++)
        {
            t_iface        ifc;

            ft_bzero(&ifc, sizeof(ifc));
            ifc.name = ft_strdup(subs[s].ifaces[i]);
            ifc.ip = subs[s].ips[i];
            ifc.has_ip = 1;
            ifc.mask = subs[s].mask;
            ifc.has_mask = 1;
            if (out->n_ifs >= out->cap_ifs)
            {
                out->cap_ifs = out->cap_ifs ? out->cap_ifs * 2 : 8;
                out->ifs = (t_iface *)xrealloc(out->ifs,
                        out->cap_ifs * sizeof(t_iface));
            }
            out->ifs[out->n_ifs++] = ifc;
        }
    }

    /* ---- tables de routage : une route par défaut par device ---- */
    for (d = 0; d < lvl->n_devices; d++)
    {
        const t_device_def        *dev = &lvl->devices[d];
        int            ds[16];
        int            nd = 0;
        int            g;
        int            target = -1;
        t_rt_table        rt;

        ft_bzero(&rt, sizeof(rt));
        rt.name = ft_strdup(dev->rt_name ? dev->rt_name : dev->name);
        rt.capacity = 4;
        rt.entries = (t_rt_entry *)xcalloc(4, sizeof(t_rt_entry));
        dev_subnets(lvl, subs, n_subs, d, ds, &nd);

        /* 1) vers la destination d'un objectif (garantit l'objectif) */
        for (g = 0; g < lvl->n_goals && target < 0; g++)
        {
            const t_goal        *gl = &lvl->goals[g];
            int            dd = device_index_name(lvl, gl->dst);
            int            dsd[16];
            int            ndd = 0;
            int            k;

            if (dd < 0 || dd == d)
                continue;
            dev_subnets(lvl, subs, n_subs, dd, dsd, &ndd);
            for (k = 0; k < ndd; k++)
            {
                if (!subnet_of(ds, nd, dsd[k]))
                {
                    target = dsd[k];
                    break;
                }
            }
        }
        /* 2) sinon vers la source (permet le retour bidirectionnel) */
        if (target < 0)
        {
            for (g = 0; g < lvl->n_goals && target < 0; g++)
            {
                const t_goal        *gl = &lvl->goals[g];
                int            sd = device_index_name(lvl, gl->src);
                int            dss[16];
                int            nss = 0;
                int            k;

                if (sd < 0 || sd == d)
                    continue;
                dev_subnets(lvl, subs, n_subs, sd, dss, &nss);
                for (k = 0; k < nss; k++)
                {
                    if (!subnet_of(ds, nd, dss[k]))
                    {
                        target = dss[k];
                        break;
                    }
                }
            }
        }
        if (target >= 0)
        {
            t_rt_entry        e;
            t_ip            gate;

            ft_bzero(&e, sizeof(e));
            e.dest = 0;
            e.dest_mask = 0;
            e.has_route = 1;
            gate = next_hop_ip(lvl, subs, n_subs, d, ds[0], target);
            if (gate)
            {
                e.gateway = gate;
                e.has_gate = 1;
            }
            rt.entries[rt.count++] = e;
        }
        if (out->n_rts >= out->cap_rts)
        {
            out->cap_rts = out->cap_rts ? out->cap_rts * 2 : 8;
            out->rts = (t_rt_table *)xrealloc(out->rts,
                    out->cap_rts * sizeof(t_rt_table));
        }
        out->rts[out->n_rts++] = rt;
    }

    ft_free(adj);
    for (s = 0; s < n_subs; s++)
    {
        for (i = 0; i < subs[s].n; i++)
            ft_free(subs[s].ifaces[i]);
        ft_free(subs[s].ifaces);
        ft_free(subs[s].dev_idx);
        ft_free(subs[s].ips);
    }
    ft_free(subs);
    (void)k;
    return (0);
}
