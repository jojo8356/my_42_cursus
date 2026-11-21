/* ************************************************************************** */
/*                                                                            */
/*   test_routing.c                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests : plus long préfixe correspondant.                              */
/* ************************************************************************** */

# include "test_framework.h"

TEST(test_default_route)
{
    t_route        routes[1];
    t_ip            ip;

    routes[0].dest = 0;
    routes[0].dest_mask = 0;
    routes[0].gateway = 0x0A000001u;
    ip_parse("1.2.3.4", &ip);
    ASSERT_EQ_INT(route_match(routes, 1, ip), 0, "default matche tout");
    ip_parse("255.255.255.255", &ip);
    ASSERT_EQ_INT(route_match(routes, 1, ip), 0, "default matche 255.255.255.255");
}

TEST(test_specific_route)
{
    t_route        routes[2];
    t_ip            ip;

    /* 10.0.0.0/8 et 10.1.0.0/16 */
    routes[0].dest = 0x0A000000u;
    routes[0].dest_mask = cidr_to_mask(8);
    routes[0].gateway = 0x0A000001u;
    routes[1].dest = 0x0A010000u;
    routes[1].dest_mask = cidr_to_mask(16);
    routes[1].gateway = 0x0A010002u;

    ip_parse("10.1.2.3", &ip);
    /* les deux correspondent ; le plus long (/16) doit l'emporter */
    ASSERT_EQ_INT(route_match(routes, 2, ip), 1, "plus long préfixe /16");
    ip_parse("10.2.3.4", &ip);
    ASSERT_EQ_INT(route_match(routes, 2, ip), 0, "seul /8 correspond");
    ip_parse("192.168.1.1", &ip);
    ASSERT_EQ_INT(route_match(routes, 2, ip), -1, "aucune route");
}

TEST(test_no_match)
{
    t_route        routes[1];
    t_ip            ip;

    routes[0].dest = 0x0A000000u;
    routes[0].dest_mask = cidr_to_mask(8);
    ip_parse("172.16.0.1", &ip);
    ASSERT_EQ_INT(route_match(routes, 1, ip), -1, "pas de correspondance");
}

TEST(test_empty_routes)
{
    t_ip            ip;
    ip_parse("1.2.3.4", &ip);
    ASSERT_EQ_INT(route_match(NULL, 0, ip), -1, "table vide");
}

int            main(void)
{
    RUN(test_default_route);
    RUN(test_specific_route);
    RUN(test_no_match);
    RUN(test_empty_routes);
    SUMMARY();
}
