/* ************************************************************************** */
/*                                                                            */
/*   json.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Parseur JSON minimal (null/bool/num/string/array/object).           */
/* ************************************************************************** */

# include "netpratice.h"

typedef struct s_parser
{
    const char        *s;
    int                pos;
    int                len;
}                   t_parser;

static void        skip_ws(t_parser *p)
{
    while (p->pos < p->len && (p->s[p->pos] == ' ' || p->s[p->pos] == '\t'
        || p->s[p->pos] == '\n' || p->s[p->pos] == '\r'))
        p->pos++;
}

static t_json    *parse_value(t_parser *p);

static char        *parse_string_raw(t_parser *p)
{
    /* p->s[p->pos] == '"' */
    int            start = p->pos + 1;
    int            i = start;
    char            *out;
    int            len = 0;

    while (i < p->len && p->s[i] != '"')
    {
        if (p->s[i] == '\\')
        {
            i++;
            if (i >= p->len)
                return (NULL);
            if (p->s[i] == 'n')
                len += 1;
            else if (p->s[i] == 't')
                len += 1;
            else if (p->s[i] == 'r')
                len += 1;
            else if (p->s[i] == 'b')
                len += 1;
            else if (p->s[i] == 'f')
                len += 1;
            else
                len += 1;
            i++;
        }
        else
        {
            len++;
            i++;
        }
    }
    if (i >= p->len)
        return (NULL);
    out = (char *)xmalloc(len + 1);
    ft_bzero(out, len + 1);
    i = start;
    len = 0;
    while (i < p->len && p->s[i] != '"')
    {
        if (p->s[i] == '\\')
        {
            i++;
            char            c = p->s[i];
            if (c == 'n')
                c = '\n';
            else if (c == 't')
                c = '\t';
            else if (c == 'r')
                c = '\r';
            else if (c == 'b')
                c = '\b';
            else if (c == 'f')
                c = '\f';
            out[len++] = c;
            i++;
        }
        else
        {
            out[len++] = p->s[i];
            i++;
        }
    }
    out[len] = '\0';
    p->pos = i + 1;
    return (out);
}

static t_json    *parse_string(t_parser *p)
{
    char            *s = parse_string_raw(p);
    t_json            *j;

    if (!s)
        return (NULL);
    j = (t_json *)xcalloc(1, sizeof(t_json));
    j->type = JSON_STRING;
    j->str = s;
    return (j);
}

static double   parse_double_local(const char *s)
{
    double            val = 0.0;
    int                sign = 1;
    int                i = 0;

    if (s[i] == '-')
    {
        sign = -1;
        i++;
    }
    while (s[i] >= '0' && s[i] <= '9')
    {
        val = val * 10.0 + (s[i] - '0');
        i++;
    }
    if (s[i] == '.')
    {
        i++;
        double            frac = 0.0;
        double            scale = 0.1;
        while (s[i] >= '0' && s[i] <= '9')
        {
            frac += (s[i] - '0') * scale;
            scale *= 0.1;
            i++;
        }
        val += frac;
    }
    return (val * sign);
}

static t_json    *parse_number(t_parser *p)
{
    int            start = p->pos;
    t_json            *j;

    if (p->s[p->pos] == '-')
        p->pos++;
    while (p->pos < p->len && (ft_isdigit(p->s[p->pos])
        || p->s[p->pos] == '.' || p->s[p->pos] == 'e' || p->s[p->pos] == 'E'
        || p->s[p->pos] == '+' || p->s[p->pos] == '-'))
        p->pos++;
    j = (t_json *)xcalloc(1, sizeof(t_json));
    j->type = JSON_NUMBER;
    j->num = parse_double_local(&p->s[start]);
    return (j);
}

static t_json    *parse_array(t_parser *p)
{
    /* p->s[p->pos] == '[' */
    t_json            *j;
    t_json            *elem;

    p->pos++;
    j = (t_json *)xcalloc(1, sizeof(t_json));
    j->type = JSON_ARRAY;
    j->children = (t_json **)xcalloc(4, sizeof(t_json *));
    j->cap = 4;
    j->count = 0;
    skip_ws(p);
    if (p->pos < p->len && p->s[p->pos] == ']')
    {
        p->pos++;
        return (j);
    }
    while (p->pos < p->len)
    {
        skip_ws(p);
        elem = parse_value(p);
        if (!elem)
            return (NULL);
        if (j->count >= j->cap)
        {
            j->cap *= 2;
            j->children = (t_json **)xrealloc(j->children,
                    j->cap * sizeof(t_json *));
        }
        j->children[j->count++] = elem;
        skip_ws(p);
        if (p->pos < p->len && p->s[p->pos] == ',')
        {
            p->pos++;
            continue;
        }
        if (p->pos < p->len && p->s[p->pos] == ']')
        {
            p->pos++;
            return (j);
        }
        return (NULL);
    }
    return (NULL);
}

static t_json    *parse_object(t_parser *p)
{
    /* p->s[p->pos] == '{' */
    t_json            *j;
    char            *key;
    t_json            *val;

    p->pos++;
    j = (t_json *)xcalloc(1, sizeof(t_json));
    j->type = JSON_OBJECT;
    j->children = (t_json **)xcalloc(4, sizeof(t_json *));
    j->keys = (char **)xcalloc(4, sizeof(char *));
    j->cap = 4;
    j->count = 0;
    skip_ws(p);
    if (p->pos < p->len && p->s[p->pos] == '}')
    {
        p->pos++;
        return (j);
    }
    while (p->pos < p->len)
    {
        skip_ws(p);
        if (p->pos >= p->len || p->s[p->pos] != '"')
            return (NULL);
        key = parse_string_raw(p);
        if (!key)
            return (NULL);
        skip_ws(p);
        if (p->pos >= p->len || p->s[p->pos] != ':')
        {
            ft_free(key);
            return (NULL);
        }
        p->pos++;
        skip_ws(p);
        val = parse_value(p);
        if (!val)
        {
            ft_free(key);
            return (NULL);
        }
        if (j->count >= j->cap)
        {
            j->cap *= 2;
            j->children = (t_json **)xrealloc(j->children,
                    j->cap * sizeof(t_json *));
            j->keys = (char **)xrealloc(j->keys, j->cap * sizeof(char *));
        }
        j->keys[j->count] = key;
        j->children[j->count] = val;
        j->count++;
        skip_ws(p);
        if (p->pos < p->len && p->s[p->pos] == ',')
        {
            p->pos++;
            continue;
        }
        if (p->pos < p->len && p->s[p->pos] == '}')
        {
            p->pos++;
            return (j);
        }
        return (NULL);
    }
    return (NULL);
}

static t_json    *parse_value(t_parser *p)
{
    skip_ws(p);
    if (p->pos >= p->len)
        return (NULL);
    char            c = p->s[p->pos];
    if (c == '"')
        return (parse_string(p));
    if (c == '{')
        return (parse_object(p));
    if (c == '[')
        return (parse_array(p));
    if (c == 't')
    {
        if (ft_strncmp(p->s + p->pos, "true", 4) == 0)
        {
            p->pos += 4;
            t_json            *j = (t_json *)xcalloc(1, sizeof(t_json));
            j->type = JSON_BOOL;
            j->boolean = 1;
            return (j);
        }
        return (NULL);
    }
    if (c == 'f')
    {
        if (ft_strncmp(p->s + p->pos, "false", 5) == 0)
        {
            p->pos += 5;
            t_json            *j = (t_json *)xcalloc(1, sizeof(t_json));
            j->type = JSON_BOOL;
            j->boolean = 0;
            return (j);
        }
        return (NULL);
    }
    if (c == 'n')
    {
        if (ft_strncmp(p->s + p->pos, "null", 4) == 0)
        {
            p->pos += 4;
            t_json            *j = (t_json *)xcalloc(1, sizeof(t_json));
            j->type = JSON_NULL;
            return (j);
        }
        return (NULL);
    }
    if (c == '-' || ft_isdigit(c))
        return (parse_number(p));
    return (NULL);
}

t_json            *json_parse(const char *s)
{
    t_parser        p;
    t_json            *j;

    if (!s)
        return (NULL);
    ft_bzero(&p, sizeof(p));
    p.s = s;
    p.len = (int)ft_strlen(s);
    p.pos = 0;
    j = parse_value(&p);
    if (!j)
        return (NULL);
    skip_ws(&p);
    if (p.pos != p.len)
    {
        /* tolerance : on accepte le premier valeur, mais on vérifie
           que le reste n'est que du blanc. Sinon rejet strict. */
        return (NULL);
    }
    return (j);
}

void            json_free(t_json *j)
{
    int            i;

    if (!j)
        return;
    if (j->type == JSON_STRING)
        ft_free(j->str);
    else if (j->type == JSON_ARRAY || j->type == JSON_OBJECT)
    {
        for (i = 0; i < j->count; i++)
        {
            if (j->type == JSON_OBJECT)
                ft_free(j->keys[i]);
            json_free(j->children[i]);
        }
        ft_free(j->children);
        ft_free(j->keys);
    }
    ft_free(j);
}

t_json            *json_get(const t_json *obj, const char *key)
{
    int            i;

    if (!obj || obj->type != JSON_OBJECT || !key)
        return (NULL);
    for (i = 0; i < obj->count; i++)
    {
        if (ft_strcmp(obj->keys[i], key) == 0)
            return (obj->children[i]);
    }
    return (NULL);
}

t_json            *json_at(const t_json *arr, int i)
{
    if (!arr || arr->type != JSON_ARRAY || i < 0 || i >= arr->count)
        return (NULL);
    return (arr->children[i]);
}
