/* ************************************************************************** */
/*                                                                            */
/*   level.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Parseur de définition de niveau (topologie + objectifs).             */
/* ************************************************************************** */

# include "netpratice.h"

static char        *level_read_file(const char *path)
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

static int        add_device(t_level *lvl, const t_json *dj)
{
    t_device_def        dev;
    t_json            *name_j;
    t_json            *ifs_j;
    t_json            *rt_j;
    int            i;

    ft_bzero(&dev, sizeof(dev));
    name_j = json_get(dj, "name");
    if (!name_j || name_j->type != JSON_STRING)
        return (-1);
    dev.name = ft_strdup(name_j->str);
    ifs_j = json_get(dj, "ifs");
    if (ifs_j && ifs_j->type == JSON_ARRAY)
    {
        dev.n_ifs = ifs_j->count;
        dev.if_names = (char **)xcalloc(ifs_j->count ? ifs_j->count : 1,
                sizeof(char *));
        for (i = 0; i < ifs_j->count; i++)
        {
            t_json            *el = json_at(ifs_j, i);
            if (el && el->type == JSON_STRING)
                dev.if_names[i] = ft_strdup(el->str);
        }
    }
    rt_j = json_get(dj, "rt");
    if (rt_j && rt_j->type == JSON_STRING)
        dev.rt_name = ft_strdup(rt_j->str);
    if (lvl->n_devices >= 0)
    {
        int            cap = lvl->n_devices + 1;
        t_device_def        *tmp = (t_device_def *)xmalloc(cap * sizeof(t_device_def));
        ft_memcpy(tmp, lvl->devices, lvl->n_devices * sizeof(t_device_def));
        ft_free(lvl->devices);
        lvl->devices = tmp;
    }
    lvl->devices[lvl->n_devices++] = dev;
    return (0);
}

static int        add_goal(t_level *lvl, const t_json *gj)
{
    t_goal            goal;
    t_json            *src_j;
    t_json            *dst_j;

    ft_bzero(&goal, sizeof(goal));
    src_j = json_get(gj, "src");
    dst_j = json_get(gj, "dst");
    if (!src_j || src_j->type != JSON_STRING || !dst_j || dst_j->type != JSON_STRING)
        return (-1);
    goal.src = ft_strdup(src_j->str);
    goal.dst = ft_strdup(dst_j->str);
    if (lvl->n_goals > 0)
    {
        int            cap = lvl->n_goals + 1;
        t_goal            *tmp = (t_goal *)xmalloc(cap * sizeof(t_goal));
        ft_memcpy(tmp, lvl->goals, lvl->n_goals * sizeof(t_goal));
        ft_free(lvl->goals);
        lvl->goals = tmp;
    }
    else
    {
        lvl->goals = (t_goal *)xmalloc(sizeof(t_goal));
    }
    lvl->goals[lvl->n_goals++] = goal;
    return (0);
}

static int        add_link(t_level *lvl, const t_json *lj)
{
    t_link            link;
    const t_json        *ifs_j;
    int            i;

    ft_bzero(&link, sizeof(link));
    ifs_j = lj;
    if (!ifs_j || ifs_j->type != JSON_ARRAY)
        return (-1);
    link.n_ifs = ifs_j->count;
    link.ifs = (char **)xcalloc(ifs_j->count ? ifs_j->count : 1,
            sizeof(char *));
    for (i = 0; i < ifs_j->count; i++)
    {
        t_json            *el = json_at(ifs_j, i);
        if (el && el->type == JSON_STRING)
            link.ifs[i] = ft_strdup(el->str);
    }
    if (lvl->n_links > 0)
    {
        int            cap = lvl->n_links + 1;
        t_link            *tmp = (t_link *)xmalloc(cap * sizeof(t_link));
        ft_memcpy(tmp, lvl->links, lvl->n_links * sizeof(t_link));
        ft_free(lvl->links);
        lvl->links = tmp;
    }
    else
    {
        lvl->links = (t_link *)xmalloc(sizeof(t_link));
    }
    lvl->links[lvl->n_links++] = link;
    return (0);
}

int            level_parse_str(const char *s, t_level *lvl)
{
    t_json            *j;
    t_json            *devices_j;
    t_json            *goals_j;
    t_json            *name_j;
    int            i;

    if (!s || !lvl)
        return (-1);
    ft_bzero(lvl, sizeof(*lvl));
    j = json_parse(s);
    if (!j || j->type != JSON_OBJECT)
    {
        if (j)
            json_free(j);
        return (-1);
    }
    name_j = json_get(j, "name");
    if (name_j && name_j->type == JSON_STRING)
        lvl->name = ft_strdup(name_j->str);
    devices_j = json_get(j, "devices");
    if (devices_j && devices_j->type == JSON_ARRAY)
    {
        lvl->devices = (t_device_def *)xcalloc(devices_j->count ? devices_j->count : 1,
                sizeof(t_device_def));
        for (i = 0; i < devices_j->count; i++)
        {
            t_json            *dj = json_at(devices_j, i);
            if (dj && add_device(lvl, dj) != 0)
            {
                level_free(lvl);
                json_free(j);
                return (-1);
            }
        }
    }
    goals_j = json_get(j, "goals");
    if (goals_j && goals_j->type == JSON_ARRAY)
    {
        for (i = 0; i < goals_j->count; i++)
        {
            t_json            *gj = json_at(goals_j, i);
            if (gj && add_goal(lvl, gj) != 0)
            {
                level_free(lvl);
                json_free(j);
                return (-1);
            }
        }
    }
    {
        t_json            *links_j = json_get(j, "links");
        if (links_j && links_j->type == JSON_ARRAY)
        {
            for (i = 0; i < links_j->count; i++)
            {
                t_json            *lj = json_at(links_j, i);
                if (lj && add_link(lvl, lj) != 0)
                {
                    level_free(lvl);
                    json_free(j);
                    return (-1);
                }
            }
        }
    }
    json_free(j);
    return (0);
}

int            level_parse_file(const char *path, t_level *lvl)
{
    char            *buf = level_read_file(path);
    int            ret;

    if (!buf)
        return (-1);
    ret = level_parse_str(buf, lvl);
    ft_free(buf);
    return (ret);
}

void            level_free(t_level *lvl)
{
    int            i;
    int            k;

    if (!lvl)
        return;
    ft_free(lvl->name);
    for (i = 0; i < lvl->n_devices; i++)
    {
        ft_free(lvl->devices[i].name);
        ft_free(lvl->devices[i].rt_name);
        for (k = 0; k < lvl->devices[i].n_ifs; k++)
            ft_free(lvl->devices[i].if_names[k]);
        ft_free(lvl->devices[i].if_names);
    }
    ft_free(lvl->devices);
    for (i = 0; i < lvl->n_goals; i++)
    {
        ft_free(lvl->goals[i].src);
        ft_free(lvl->goals[i].dst);
    }
    ft_free(lvl->goals);
    for (i = 0; i < lvl->n_links; i++)
    {
        int            k;
        for (k = 0; k < lvl->links[i].n_ifs; k++)
            ft_free(lvl->links[i].ifs[k]);
        ft_free(lvl->links[i].ifs);
    }
    ft_free(lvl->links);
    ft_bzero(lvl, sizeof(*lvl));
}
