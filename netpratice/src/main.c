/* ************************************************************************** */
/*                                                                            */
/*   main.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   CLI NetPratice : solve / validate / show.                             */
/* ************************************************************************** */

# include "netpratice.h"

static void        print_usage(void)
{
    ft_putstr_fd("NetPratice — engine IPv4 / sous-réseaux / routage\n", 1);
    ft_putstr_fd("Usage:\n", 1);
    ft_putstr_fd("  netpratice solve <level.json> [out.json]\n", 1);
    ft_putstr_fd("  netpratice validate <level.json> <solution.json>\n", 1);
    ft_putstr_fd("  netpratice show <solution.json>\n", 1);
    ft_putstr_fd("  netpratice cidr <ip/mask>   (affiche réseau/broadcast)\n", 1);
}

static int        cmd_cidr(const char *arg)
{
    t_ip            ip;
    uint32_t        mask;
    t_subnet        s;
    char            *ipstr;
    char            *netstr;
    char            *bcstr;

    if (ft_strchr(arg, '/') != NULL)
    {
        if (ip_parse(arg, &ip) != 0)
        {
            ft_putstr_fd("IP invalide\n", 2);
            return (1);
        }
        mask = cidr_to_mask(ft_atoi(ft_strchr(arg, '/') + 1));
    }
    else
    {
        const char            *sp = ft_strchr(arg, ' ');
        if (!sp)
        {
            ft_putstr_fd("format: <ip> <mask> ou <ip/cidr>\n", 2);
            return (1);
        }
        if (ip_parse(arg, &ip) != 0 || mask_parse(sp + 1, &mask) != 0)
        {
            ft_putstr_fd("IP/masque invalide\n", 2);
            return (1);
        }
    }
    s = subnet_new(ip, mask);
    ipstr = ip_to_str(ip);
    netstr = ip_to_str(s.network);
    bcstr = ip_to_str(subnet_broadcast(&s));
    ft_putstr_fd("IP        : ", 1);
    ft_putstr_fd(ipstr, 1);
    ft_putchar_fd('\n', 1);
    ft_putstr_fd("Masque    : ", 1);
    {
        char            m[32];
        int            cidr = mask_to_cidr(mask);
        if (cidr >= 0)
            ft_snprintf(m, sizeof(m), "/%d (%s)", cidr, ip_to_str(mask));
        else
            ft_snprintf(m, sizeof(m), "%s", ip_to_str(mask));
        ft_putstr_fd(m, 1);
    }
    ft_putchar_fd('\n', 1);
    ft_putstr_fd("Réseau    : ", 1);
    ft_putstr_fd(netstr, 1);
    ft_putchar_fd('\n', 1);
    ft_putstr_fd("Broadcast : ", 1);
    ft_putstr_fd(bcstr, 1);
    ft_putchar_fd('\n', 1);
    ft_putstr_fd("Hôtes     : ", 1);
    {
        char            h[32];
        ft_snprintf(h, sizeof(h), "%d\n", subnet_num_hosts(&s));
        ft_putstr_fd(h, 1);
    }
    ft_free(ipstr);
    ft_free(netstr);
    ft_free(bcstr);
    return (0);
}

static int        cmd_solve(const char *level_path, const char *out_path)
{
    t_level            lvl;
    t_solution        sol;
    t_topo            topo;
    char            *json;
    char            *err = NULL;

    ft_bzero(&topo, sizeof(topo));
    if (level_parse_file(level_path, &lvl) != 0)
    {
        ft_putstr_fd("échec lecture du niveau\n", 2);
        return (1);
    }
    if (solver_solve(&lvl, &topo, &sol) != 0)
    {
        ft_putstr_fd("échec du solveur\n", 2);
        level_free(&lvl);
        return (1);
    }
    if (solution_validate_goals(&sol, &lvl, &err) != 0)
    {
        ft_putstr_fd("solveur : solution non valide : ", 2);
        if (err)
            ft_putstr_fd(err, 2);
        ft_putchar_fd('\n', 2);
        ft_free(err);
    }
    else
    {
        ft_putstr_fd("solveur : solution valide ✓\n", 1);
    }
    json = solution_to_json(&sol);
    if (out_path)
    {
        FILE            *f = fopen(out_path, "w");
        if (f)
        {
            fputs(json, f);
            fclose(f);
            ft_putstr_fd("écriture : ", 1);
            ft_putstr_fd((char *)out_path, 1);
            ft_putchar_fd('\n', 1);
        }
    }
    else
    {
        ft_putstr_fd(json, 1);
        ft_putchar_fd('\n', 1);
    }
    ft_free(json);
    solution_free(&sol);
    level_free(&lvl);
    return (0);
}

static int        cmd_validate(const char *level_path, const char *sol_path)
{
    t_level            lvl;
    t_solution        sol;
    char            *err = NULL;

    if (level_parse_file(level_path, &lvl) != 0)
    {
        ft_putstr_fd("échec lecture du niveau\n", 2);
        return (1);
    }
    if (solution_parse_file(sol_path, &sol) != 0)
    {
        ft_putstr_fd("échec lecture de la solution\n", 2);
        level_free(&lvl);
        return (1);
    }
    if (solution_validate_goals(&sol, &lvl, &err) == 0)
    {
        ft_putstr_fd("VALIDE ✓ — tous les objectifs atteints\n", 1);
        solution_free(&sol);
        level_free(&lvl);
        return (0);
    }
    ft_putstr_fd("NON VALIDE ✗ : ", 2);
    if (err)
        ft_putstr_fd(err, 2);
    ft_putchar_fd('\n', 2);
    ft_free(err);
    solution_free(&sol);
    level_free(&lvl);
    return (1);
}

static int        cmd_show(const char *sol_path)
{
    t_solution        sol;
    char            *json;

    if (solution_parse_file(sol_path, &sol) != 0)
    {
        ft_putstr_fd("échec lecture de la solution\n", 2);
        return (1);
    }
    json = solution_to_json(&sol);
    ft_putstr_fd(json, 1);
    ft_putchar_fd('\n', 1);
    ft_free(json);
    solution_free(&sol);
    return (0);
}

int            main(int argc, char **argv)
{
    if (argc < 2)
    {
        print_usage();
        return (1);
    }
    if (ft_strcmp(argv[1], "cidr") == 0 && argc >= 3)
        return (cmd_cidr(argv[2]));
    if (ft_strcmp(argv[1], "solve") == 0 && argc >= 3)
        return (cmd_solve(argv[2], argc >= 4 ? argv[3] : NULL));
    if (ft_strcmp(argv[1], "validate") == 0 && argc >= 4)
        return (cmd_validate(argv[2], argv[3]));
    if (ft_strcmp(argv[1], "show") == 0 && argc >= 3)
        return (cmd_show(argv[2]));
    print_usage();
    return (1);
}
