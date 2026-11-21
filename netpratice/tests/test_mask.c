/* ************************************************************************** */
/*                                                                            */
/*   test_mask.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests : masques, CIDR, réseau/broadcast/hôtes, chevauchement.        */
/* ************************************************************************** */

# include "test_framework.h"

TEST(test_cidr_table)
{
    /* table de référence CIDR / masque / bloc / hôtes */
    struct { int cidr; const char *mask; int hosts; } t[] = {
        {30, "255.255.255.252", 2},
        {29, "255.255.255.248", 6},
        {28, "255.255.255.240", 14},
        {27, "255.255.255.224", 30},
        {26, "255.255.255.192", 62},
        {25, "255.255.255.128", 126},
        {24, "255.255.255.0", 254},
        {16, "255.255.0.0", 65534},
    };
    int            i;
    t_ip            ip;
    uint32_t        mask;

    for (i = 0; i < 8; i++)
    {
        ip_parse("10.0.0.1", &ip);
        mask = cidr_to_mask(t[i].cidr);
        ASSERT(mask_is_valid(mask), "masque valide");
        ASSERT_EQ_INT(mask_to_cidr(mask), t[i].cidr, "cidr");
        {
            t_subnet        tmp = subnet_new(ip, mask);
            ASSERT_EQ_INT(subnet_num_hosts(&tmp), t[i].hosts, "nb hôtes");
        }
        {
            char            *ms = ip_to_str(mask);
            ASSERT_EQ_STR(ms, t[i].mask, "masque décimal");
            ft_free(ms);
        }
    }
}

TEST(test_mask_parse_forms)
{
    uint32_t        mask = 0;
    ASSERT_EQ_INT(mask_parse("/24", &mask), 0, "parse /24");
    ASSERT_EQ_INT(mask, 0xFFFFFF00u, "valeur /24");
    ASSERT_EQ_INT(mask_parse("255.255.255.0", &mask), 0, "parse décimal");
    ASSERT_EQ_INT(mask, 0xFFFFFF00u, "valeur décimal");
    ASSERT_EQ_INT(mask_parse("255.255.255.252", &mask), 0, "parse /30");
    ASSERT_EQ_INT(mask_to_cidr(mask), 30, "/30");
}

TEST(test_mask_invalid)
{
    uint32_t        mask = 0;
    ASSERT_EQ_INT(mask_parse("255.0.255.0", &mask), -1, "masque non contigu");
    ASSERT_EQ_INT(mask_parse("/33", &mask), -1, "cidr >32");
    ASSERT_EQ_INT(mask_parse("/-1", &mask), -1, "cidr <0");
    ASSERT_EQ_INT(mask_parse("abc", &mask), -1, "non numérique");
    ASSERT(!mask_is_valid(0xFFFFFF01u), "non contigu invalide");
    ASSERT(mask_is_valid(0), "mask 0 valide");
    ASSERT(mask_is_valid(0xFFFFFFFFu), "mask /32 valide");
}

TEST(test_network_broadcast)
{
    t_ip            ip;
    t_subnet        s;
    ip_parse("192.168.1.50", &ip);
    s = subnet_new(ip, cidr_to_mask(26));
    {
        char            *n = ip_to_str(s.network);
        char            *b = ip_to_str(subnet_broadcast(&s));
        ASSERT_EQ_STR(n, "192.168.1.0", "réseau /26");
        ASSERT_EQ_STR(b, "192.168.1.63", "broadcast /26");
        ft_free(n);
        ft_free(b);
    }
    ip_parse("10.1.2.3", &ip);
    s = subnet_new(ip, cidr_to_mask(30));
    {
        char            *n = ip_to_str(s.network);
        char            *b = ip_to_str(subnet_broadcast(&s));
        ASSERT_EQ_STR(n, "10.1.2.0", "réseau /30");
        ASSERT_EQ_STR(b, "10.1.2.3", "broadcast /30");
        ft_free(n);
        ft_free(b);
    }
}

TEST(test_contains_and_overlap)
{
    t_ip            a, b, c;
    t_subnet        s1, s2, s3;
    ip_parse("10.0.0.5", &a);
    ip_parse("10.0.0.200", &b);
    ip_parse("10.0.1.1", &c);
    s1 = subnet_new(a, cidr_to_mask(24));
    s2 = subnet_new(b, cidr_to_mask(24));
    s3 = subnet_new(c, cidr_to_mask(24));
    ASSERT(subnet_contains(&s1, a), "a dans s1");
    ASSERT(subnet_contains(&s1, b), "b dans s1");
    ASSERT(!subnet_contains(&s1, c), "c pas dans s1");
    ASSERT(subnets_overlap(&s1, &s2), "s1/s2 chevauchent");
    ASSERT(!subnets_overlap(&s1, &s3), "s1/s3 ne chevauchent pas");
    /* chevauchement partiel */
    {
        t_subnet        s4 = subnet_new(a, cidr_to_mask(25));
        t_subnet        s5 = subnet_new(b, cidr_to_mask(25));
        ASSERT(!subnets_overlap(&s4, &s5), "sous-réseaux /25 disjoints");
    }
}

int            main(void)
{
    RUN(test_cidr_table);
    RUN(test_mask_parse_forms);
    RUN(test_mask_invalid);
    RUN(test_network_broadcast);
    RUN(test_contains_and_overlap);
    SUMMARY();
}
