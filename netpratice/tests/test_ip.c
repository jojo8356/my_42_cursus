/* ************************************************************************** */
/*                                                                            */
/*   test_ip.c                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests : parse/format/validité IPv4.                                   */
/* ************************************************************************** */

# include "test_framework.h"

TEST(test_parse_basic)
{
    t_ip            ip = 0;
    ASSERT_EQ_INT(ip_parse("192.168.1.1", &ip), 0, "parse ok");
    ASSERT_EQ_INT((ip >> 24) & 0xFF, 192, "octet 1");
    ASSERT_EQ_INT((ip >> 16) & 0xFF, 168, "octet 2");
    ASSERT_EQ_INT((ip >> 8) & 0xFF, 1, "octet 3");
    ASSERT_EQ_INT(ip & 0xFF, 1, "octet 4");
}

TEST(test_parse_edge)
{
    t_ip            ip = 0;
    ASSERT_EQ_INT(ip_parse("0.0.0.0", &ip), 0, "0.0.0.0");
    ASSERT_EQ_INT(ip, 0, "valeur 0");
    ASSERT_EQ_INT(ip_parse("255.255.255.255", &ip), 0, "255.255.255.255");
    ASSERT_EQ_INT(ip, 0xFFFFFFFFu, "valeur max");
}

TEST(test_parse_invalid)
{
    t_ip            ip = 0;
    ASSERT_EQ_INT(ip_parse("256.1.1.1", &ip), -1, "octet >255");
    ASSERT_EQ_INT(ip_parse("1.2.3", &ip), -1, "3 octets");
    ASSERT_EQ_INT(ip_parse("1.2.3.4.5", &ip), -1, "5 octets");
    ASSERT_EQ_INT(ip_parse("a.b.c.d", &ip), -1, "non numérique");
    ASSERT_EQ_INT(ip_parse("", &ip), -1, "vide");
    ASSERT_EQ_INT(ip_parse("1.2.3.-1", &ip), -1, "négatif");
}

TEST(test_to_str_roundtrip)
{
    const char        *tests[] = {"10.0.0.1", "172.16.254.1", "8.8.8.8",
        "203.0.113.7", "128.171.197.193"};
    int            i;

    for (i = 0; i < 5; i++)
    {
        t_ip            ip = 0;
        char            *s;
        ASSERT_EQ_INT(ip_parse(tests[i], &ip), 0, "parse");
        s = ip_to_str(ip);
        ASSERT_EQ_STR(s, tests[i], "roundtrip");
        ft_free(s);
    }
}

TEST(test_is_valid)
{
    t_ip            a, b;
    ip_parse("192.168.1.1", &a);
    ip_parse("255.255.255.255", &b);
    ASSERT(ip_is_valid(a), "valide");
    ASSERT(ip_is_valid(b), "valide max");
    ASSERT(ip_is_valid(0), "valide 0");
}

int            main(void)
{
    RUN(test_parse_basic);
    RUN(test_parse_edge);
    RUN(test_parse_invalid);
    RUN(test_to_str_roundtrip);
    RUN(test_is_valid);
    SUMMARY();
}
