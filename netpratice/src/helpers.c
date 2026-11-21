/* ************************************************************************** */
/*                                                                            */
/*   helpers.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Fonctions utilitaires manquantes dans la libft minimale (partie 1).   */
/* ************************************************************************** */

# include "netpratice.h"
# include <stdarg.h>
# include <stdio.h>
# include <stdlib.h>

/* ft_snprintf : équivalent de snprintf */
int            ft_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list        ap;
    int            ret;

    va_start(ap, fmt);
    ret = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return (ret);
}

/* ft_strcmp */
int            ft_strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return ((unsigned char)*s1 - (unsigned char)*s2);
}

/* ft_strtok */
char            *ft_strtok(char *str, const char *delim, char **saveptr)
{
    char            *token;

    if (str == NULL)
    {
        str = *saveptr;
    }
    if (*str == '\0')
        return (NULL);
    str += ft_strspn(str, delim);
    if (*str == '\0')
    {
        *saveptr = str;
        return (NULL);
    }
    token = str;
    str = ft_strpbrk(str, delim);
    if (str == NULL)
    {
        *saveptr = token + ft_strlen(token);
        return (token);
    }
    *str = '\0';
    *saveptr = str + 1;
    return (token);
}

/* ft_strspn */
size_t        ft_strspn(const char *s, const char *accept)
{
    size_t        count = 0;

    while (*s && ft_strchr(accept, *s))
    {
        s++;
        count++;
    }
    return (count);
}

/* ft_strpbrk */
char            *ft_strpbrk(const char *s, const char *accept)
{
    while (*s)
    {
        if (ft_strchr(accept, *s))
            return ((char *)s);
        s++;
    }
    return (NULL);
}

/* ft_free */
void            ft_free(void *ptr)
{
    if (ptr)
        free(ptr);
}

/* ---- allocations sécurisées ---- */

void            *xmalloc(size_t size)
{
    void            *p = malloc(size);

    if (!p)
    {
        ft_putstr_fd("fatal: xmalloc failed\n", 2);
        exit(1);
    }
    return (p);
}

void            *xcalloc(size_t count, size_t size)
{
    void            *p = calloc(count, size);

    if (!p)
    {
        ft_putstr_fd("fatal: xcalloc failed\n", 2);
        exit(1);
    }
    return (p);
}

void            *xrealloc(void *ptr, size_t size)
{
    void            *p = realloc(ptr, size);

    if (!p)
    {
        ft_putstr_fd("fatal: xrealloc failed\n", 2);
        exit(1);
    }
    return (p);
}
