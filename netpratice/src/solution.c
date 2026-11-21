/* ************************************************************************** */
/*                                                                            */
/*   solution.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Conversion JSON <-> t_solution (routes + ifs).                      */
/* ************************************************************************** */

# include "netpratice.h"

/* ---- accumulateur de string ---- */

typedef struct s_sb
{
    char            *buf;
    size_t        len;
    size_t        cap;
}                   t_sb;

static void        sb_init(t_sb *sb)
{
    sb->cap = 4096;
    sb->len = 0;
    sb->buf = (char *)xmalloc(sb->cap);
    ft_bzero(sb->buf, sb->cap);
}

static void        sb_append(t_sb *sb, const char *s)
{
    size_t        ls = ft_strlen(s);

    if (sb->len + ls + 1 >= sb->cap)
    {
        while (sb->len + ls + 1 >= sb->cap)
            sb->cap *= 2;
        sb->buf = (char *)xrealloc(sb->buf, sb->cap);
    }
    ft_memcpy(sb->buf + sb->len, s, ls);
    sb->len += ls;
    sb->buf[sb->len] = '\0';
}

static void        sb_append_char(t_sb *sb, char c)
{
    char            small[2];

    small[0] = c;
    small[1] = '\0';
    sb_append(sb, small);
}

/* ---- ajout d'interfaces / tables ---- */

static void        add_iface(t_solution *sol, const char *name,
                const t_json *val)
{
    t_iface        iface;

    ft_bzero(&iface, sizeof(iface));
    iface.name = ft_strdup(name);
    if (!iface.name)
        return;
    if (val && val->type == JSON_OBJECT)
    {
        t_json            *ip_j = json_get(val, "ip");
        t_json            *mask_j = json_get(val, "mask");

        if (ip_j && ip_j->type == JSON_STRING && ip_parse(ip_j->str, &iface.ip) == 0)
            iface.has_ip = 1;
        else if (ip_j && ip_j->type == JSON_STRING)
            sol->parse_ok = 0;
        if (mask_j && mask_j->type == JSON_STRING)
        {
            if (mask_parse(mask_j->str, &iface.mask) == 0)
            {
                iface.has_mask = 1;
                iface.mask_str = ft_strdup(mask_j->str);
            }
            else
                sol->parse_ok = 0;
        }
    }
    if (sol->n_ifs >= sol->cap_ifs)
    {
        sol->cap_ifs = sol->cap_ifs ? sol->cap_ifs * 2 : 8;
        sol->ifs = (t_iface *)xrealloc(sol->ifs,
                sol->cap_ifs * sizeof(t_iface));
    }
    sol->ifs[sol->n_ifs++] = iface;
}

static void        add_rt_entry(t_solution *sol, t_rt_table *rt,
                const t_json *rj)
{
    t_rt_entry        e;

    ft_bzero(&e, sizeof(e));
    if (rj && rj->type == JSON_OBJECT)
    {
        t_json            *route_j = json_get(rj, "route");
        t_json            *gate_j = json_get(rj, "gate");
        const char            *slash;
        int            cidr;

        if (route_j && route_j->type == JSON_STRING)
        {
            if (ft_strcmp(route_j->str, "default") == 0)
            {
                e.has_route = 1;
                e.dest = 0;
                e.dest_mask = 0;
                e.route_str = ft_strdup(route_j->str);
            }
            else
            {
                slash = ft_strchr(route_j->str, '/');
                if (slash && ip_parse(route_j->str, &e.dest) == 0)
                {
                    cidr = ft_atoi(slash + 1);
                    if (cidr >= 0 && cidr <= 32)
                    {
                        e.dest = ip_apply_mask(e.dest, cidr_to_mask(cidr));
                        e.dest_mask = cidr_to_mask(cidr);
                        e.has_route = 1;
                        e.route_str = ft_strdup(route_j->str);
                    }
                    else
                        sol->parse_ok = 0;
                }
                else
                    sol->parse_ok = 0;
            }
        }
        if (gate_j && gate_j->type == JSON_STRING)
        {
            if (ip_parse(gate_j->str, &e.gateway) == 0)
                e.has_gate = 1;
            else
                sol->parse_ok = 0;
        }
    }
    if (rt->count >= rt->capacity)
    {
        rt->capacity *= 2;
        rt->entries = (t_rt_entry *)xrealloc(rt->entries,
                rt->capacity * sizeof(t_rt_entry));
    }
    /* ne conserver que si route ou gate present (evite {} -> entree vide) */
    if (e.has_route || e.has_gate)
        rt->entries[rt->count++] = e;
    else
        ft_free(e.route_str);
}

static void        add_rt_table(t_solution *sol, const char *name,
                const t_json *val)
{
    t_rt_table        rt;

    ft_bzero(&rt, sizeof(rt));
    rt.name = ft_strdup(name);
    if (!rt.name)
        return;
    rt.capacity = 4;
    rt.entries = (t_rt_entry *)xcalloc(4, sizeof(t_rt_entry));
    if (val && val->type == JSON_OBJECT)
    {
        /* une table de routage = une entrée (route + gate) */
        add_rt_entry(sol, &rt, val);
    }
    if (sol->n_rts >= sol->cap_rts)
    {
        sol->cap_rts = sol->cap_rts ? sol->cap_rts * 2 : 8;
        sol->rts = (t_rt_table *)xrealloc(sol->rts,
                sol->cap_rts * sizeof(t_rt_table));
    }
    sol->rts[sol->n_rts++] = rt;
}

int            solution_from_json(const t_json *j, t_solution *sol)
{
    t_json            *routes;
    t_json            *ifs;
    int            i;

    if (!j || j->type != JSON_OBJECT || !sol)
        return (-1);
    ft_bzero(sol, sizeof(*sol));
    sol->parse_ok = 1;
    routes = json_get(j, "routes");
    ifs = json_get(j, "ifs");
    if (ifs && ifs->type == JSON_OBJECT)
    {
        for (i = 0; i < ifs->count; i++)
            add_iface(sol, ifs->keys[i], ifs->children[i]);
    }
    if (routes && routes->type == JSON_OBJECT)
    {
        for (i = 0; i < routes->count; i++)
            add_rt_table(sol, routes->keys[i], routes->children[i]);
    }
    if (!sol->parse_ok)
        return (-1);
    return (0);
}

char            *solution_read_file(const char *path)
{
    char            *buf;
    size_t        cap = 4096;
    size_t        len = 0;
    size_t        n;
    FILE            *f = fopen(path, "r");

    if (!f)
        return (NULL);
    buf = (char *)xmalloc(cap);
    ft_bzero(buf, cap);
    while ((n = fread(buf + len, 1, cap - len - 1, f)) > 0)
    {
        len += n;
        if (len + 1 >= cap)
        {
            cap *= 2;
            buf = (char *)xrealloc(buf, cap);
            ft_bzero(buf + len, cap - len);
        }
    }
    buf[len] = '\0';
    fclose(f);
    return (buf);
}

int            solution_parse_file(const char *path, t_solution *sol)
{
    char            *buf = solution_read_file(path);
    t_json            *j;
    int            ret;

    if (!buf)
        return (-1);
    j = json_parse(buf);
    ft_free(buf);
    if (!j)
        return (-1);
    ret = solution_from_json(j, sol);
    json_free(j);
    return (ret);
}

int            solution_parse_str(const char *s, t_solution *sol)
{
    t_json            *j = json_parse(s);
    int            ret;

    if (!j)
        return (-1);
    ret = solution_from_json(j, sol);
    json_free(j);
    return (ret);
}

/* formatte le masque : format original si préservé, sinon CIDR. */
static void        fmt_mask(const t_iface *ifc, char *out, size_t cap)
{
    if (ifc->mask_str)
        ft_snprintf(out, cap, "%s", ifc->mask_str);
    else
    {
        int            cidr = mask_to_cidr(ifc->mask);
        if (cidr >= 0)
            ft_snprintf(out, cap, "/%d", cidr);
        else
        {
            char            *m = ip_to_str(ifc->mask);
            ft_snprintf(out, cap, "%s", m);
            ft_free(m);
        }
    }
}

char            *solution_to_json(const t_solution *sol)
{
    t_sb            sb;
    char            tmp[128];
    char            ipbuf[32];
    char            maskstr[48];
    int            i, j;

    sb_init(&sb);
    sb_append(&sb, "{\"routes\":{");
    for (i = 0; i < sol->n_rts; i++)
    {
        ft_snprintf(tmp, sizeof(tmp), "\"%s\":{", sol->rts[i].name);
        sb_append(&sb, tmp);
        for (j = 0; j < sol->rts[i].count; j++)
        {
            t_rt_entry        *e = &sol->rts[i].entries[j];
            char            routestr[64];
            int            has_part = 0;

            if (e->has_route)
            {
                if (e->route_str)
                    ft_snprintf(routestr, sizeof(routestr), "%s", e->route_str);
                else
                {
                    int            cidr = mask_to_cidr(e->dest_mask);
                    if (e->dest_mask == 0)
                        ft_snprintf(routestr, sizeof(routestr), "0.0.0.0/0");
                    else if (cidr >= 0)
                    {
                        char            base[32];
                        char            *base_str = ip_to_str(e->dest);
                        ft_snprintf(base, sizeof(base), "%s", base_str);
                        ft_snprintf(routestr, sizeof(routestr), "%s/%d",
                            base, cidr);
                        ft_free(base_str);
                    }
                    else
                        ft_snprintf(routestr, sizeof(routestr), "0.0.0.0/0");
                }
                ft_snprintf(tmp, sizeof(tmp), "\"route\":\"%s\"", routestr);
                sb_append(&sb, tmp);
                has_part = 1;
            }
            if (e->has_gate)
            {
                char            gip[32];
                char            *g = ip_to_str(e->gateway);
                if (has_part)
                    sb_append_char(&sb, ',');
                ft_snprintf(gip, sizeof(gip), "\"gate\":\"%s\"", g);
                sb_append(&sb, gip);
                ft_free(g);
            }
            if (j + 1 < sol->rts[i].count)
                sb_append_char(&sb, ',');
        }
        sb_append_char(&sb, '}');
        if (i + 1 < sol->n_rts)
            sb_append_char(&sb, ',');
    }
    sb_append(&sb, "},\"ifs\":{");
    for (i = 0; i < sol->n_ifs; i++)
    {
        t_iface        *ifc = &sol->ifs[i];

        if (!ifc->has_ip && !ifc->has_mask)
        {
            ft_snprintf(tmp, sizeof(tmp), "\"%s\":{}", ifc->name);
            sb_append(&sb, tmp);
        }
        else if (!ifc->has_ip && ifc->has_mask)
        {
            fmt_mask(ifc, maskstr, sizeof(maskstr));
            ft_snprintf(tmp, sizeof(tmp), "\"%s\":{\"mask\":\"%s\"}",
                ifc->name, maskstr);
            sb_append(&sb, tmp);
        }
        else
        {
            char            *ipstr = ip_to_str(ifc->ip);
            ft_snprintf(ipbuf, sizeof(ipbuf), "\"ip\":\"%s\"", ipstr);
            ft_free(ipstr);
            if (ifc->has_mask)
            {
                fmt_mask(ifc, maskstr, sizeof(maskstr));
                ft_snprintf(tmp, sizeof(tmp), "\"%s\":{%s,\"mask\":\"%s\"}",
                    ifc->name, ipbuf, maskstr);
            }
            else
                ft_snprintf(tmp, sizeof(tmp), "\"%s\":{%s}", ifc->name, ipbuf);
            sb_append(&sb, tmp);
        }
        if (i + 1 < sol->n_ifs)
            sb_append_char(&sb, ',');
    }
    sb_append(&sb, "}}");
    return (sb.buf);
}

void            solution_free(t_solution *sol)
{
    int            i;

    if (!sol)
        return;
    for (i = 0; i < sol->n_ifs; i++)
    {
        ft_free(sol->ifs[i].name);
        ft_free(sol->ifs[i].mask_str);
    }
    ft_free(sol->ifs);
    for (i = 0; i < sol->n_rts; i++)
    {
        int            k;
        ft_free(sol->rts[i].name);
        for (k = 0; k < sol->rts[i].count; k++)
            ft_free(sol->rts[i].entries[k].route_str);
        ft_free(sol->rts[i].entries);
    }
    ft_free(sol->rts);
    ft_bzero(sol, sizeof(*sol));
}
