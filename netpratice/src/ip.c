/* ************************************************************************** */
/*                                                                            */
/*   ip.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   IPv4 : parse, format, opérations bit à bit.                         */
/* ************************************************************************** */

# include "netpratice.h"

int            ip_parse(const char *s, t_ip *out)
{
    int            octets[4];
    int            i;
    int            n;
    char            *tok;
    char            *save;
    char            *buf;

    if (!s || !out)
        return (-1);
    buf = ft_strdup(s);
    if (!buf)
        return (-1);
    i = 0;
    tok = ft_strtok(buf, ".", &save);
    while (tok)
    {
        if (i >= 4)
        {
            ft_free(buf);
            return (-1);
        }
        if (!ft_isdigit(tok[0]) && tok[0] != '-')
        {
            ft_free(buf);
            return (-1);
        }
        n = ft_atoi(tok);
        if (n < 0 || n > 255)
        {
            ft_free(buf);
            return (-1);
        }
        octets[i++] = n;
        tok = ft_strtok(NULL, ".", &save);
    }
    ft_free(buf);
    if (i != 4)
        return (-1);
    *out = (t_ip)((octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3]);
    return (0);
}

char            *ip_to_str(t_ip ip)
{
    int            a, b, c, d;
    char            buf[32];

    a = (ip >> 24) & 0xFF;
    b = (ip >> 16) & 0xFF;
    c = (ip >> 8) & 0xFF;
    d = ip & 0xFF;
    ft_bzero(buf, sizeof(buf));
    ft_snprintf(buf, sizeof(buf), "%d.%d.%d.%d", a, b, c, d);
    return (ft_strdup(buf));
}

int            ip_is_valid(t_ip ip)
{
    int            a, b, c, d;

    a = (ip >> 24) & 0xFF;
    b = (ip >> 16) & 0xFF;
    c = (ip >> 8) & 0xFF;
    d = ip & 0xFF;
    return (a <= 255 && b <= 255 && c <= 255 && d <= 255);
}
