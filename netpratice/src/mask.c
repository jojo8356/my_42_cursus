/* ************************************************************************** */
/*                                                                            */
/*   mask.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Masques de sous-réseau, CIDR, opérations de réseau/broadcast.       */
/* ************************************************************************** */

# include "netpratice.h"

int            mask_parse(const char *s, uint32_t *out)
{
    if (!s || !out || !ft_strlen(s))
        return (-1);
    /* forme CIDR : "/24" */
    if (s[0] == '/')
    {
        int            cidr = ft_atoi(s + 1);
        if (cidr < 0 || cidr > 32)
            return (-1);
        *out = cidr_to_mask(cidr);
        return (0);
    }
    /* forme décimale : "255.255.255.0" */
    t_ip            ip;
    if (ip_parse(s, &ip) != 0)
        return (-1);
    if (!mask_is_valid(ip))
        return (-1);
    *out = ip;
    return (0);
}

int            mask_is_valid(uint32_t mask)
{
    /* un masque est une suite de 1 contigus à partir du bit de poids fort */
    uint32_t        rest = mask;

    while (rest & (1u << 31))
        rest = (rest << 1) & 0xFFFFFFFFu;
    return (rest == 0);
}

int            mask_to_cidr(uint32_t mask)
{
    int            cidr = 0;

    /* compte les 1 contigus à partir du bit de poids fort */
    while (cidr < 32 && (mask & (0x80000000u >> cidr)))
        cidr++;
    /* les bits restants (de poids faible) doivent être à 0 */
    if (cidr < 32 && (mask & ((1u << (32 - cidr)) - 1)))
        return (-1);
    return (cidr);
}

uint32_t       cidr_to_mask(int cidr)
{
    if (cidr < 0)
        cidr = 0;
    if (cidr > 32)
        cidr = 32;
    if (cidr == 0)
        return (0);
    return (0xFFFFFFFFu << (32 - cidr));
}

uint32_t       ip_apply_mask(t_ip ip, uint32_t mask)
{
    return (ip & mask);
}

/* ---- sous-réseau ---- */

t_subnet        subnet_new(t_ip ip, uint32_t mask)
{
    t_subnet        s;

    s.network = ip_apply_mask(ip, mask);
    s.mask = mask;
    return (s);
}

t_ip            subnet_network(const t_subnet *s)
{
    return (s->network);
}

t_ip            subnet_broadcast(const t_subnet *s)
{
    int            cidr = mask_to_cidr(s->mask);
    int            host_bits;
    uint32_t        block;
    t_ip            bc;

    if (cidr < 0)
        return (s->network);
    host_bits = 32 - cidr;
    block = (uint32_t)1 << host_bits;
    bc = s->network + (block - 1);
    return (bc);
}

int             subnet_num_hosts(const t_subnet *s)
{
    int            cidr = mask_to_cidr(s->mask);

    if (cidr < 0)
        return (-1);
    if (cidr >= 31)
        return (0);
    return ((1 << (32 - cidr)) - 2);
}

int             subnet_contains(const t_subnet *s, t_ip ip)
{
    return (ip_apply_mask(ip, s->mask) == s->network);
}

int             subnets_overlap(const t_subnet *a, const t_subnet *b)
{
    t_ip            n1 = a->network;
    t_ip            n2 = b->network;
    uint32_t        m;

    if (a->mask <= b->mask)
        m = a->mask;
    else
        m = b->mask;
    n1 = ip_apply_mask(n1, m);
    n2 = ip_apply_mask(n2, m);
    return (n1 == n2);
}
