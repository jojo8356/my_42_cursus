/* ************************************************************************** */
/*                                                                            */
/*   netpratice.h                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   NetPratice — engine IPv4 / sous-réseaux / routage (Rank 04, 42).      */
/* ************************************************************************** */

# ifndef NETPRACTICE_H
# define NETPRACTICE_H

# include <stddef.h>
# include <stdint.h>
# include <stdio.h>
# include "libft.h"

/* ------------------------------------------------------------------ */
/*  IPv4                                                               */
/* ------------------------------------------------------------------ */

typedef uint32_t    t_ip;

/* Parse/format */
int            ip_parse(const char *s, t_ip *out);       /* "a.b.c.d" -> 0 si ok */
char            *ip_to_str(t_ip ip);                      /* "a.b.c.d" alloue */
int            ip_is_valid(t_ip ip);                       /* octets 0..255 */

/* ------------------------------------------------------------------ */
/*  Masque / CIDR                                                       */
/* ------------------------------------------------------------------ */

/* Un masque est une suite de 1 contigus. */
int            mask_parse(const char *s, uint32_t *out);   /* "255.255.255.0" ou "/24" */
int            mask_is_valid(uint32_t mask);
int            mask_to_cidr(uint32_t mask);                /* nb de 1, -1 si invalide */
uint32_t       cidr_to_mask(int cidr);                     /* /24 -> 255.255.255.0 */
uint32_t       ip_apply_mask(t_ip ip, uint32_t mask);      /* ip & mask (adresse reseau) */

/* Sous-réseau */
typedef struct s_subnet
{
    t_ip            network;
    uint32_t        mask;
}                   t_subnet;

t_subnet        subnet_new(t_ip ip, uint32_t mask);
t_ip            subnet_network(const t_subnet *s);
t_ip            subnet_broadcast(const t_subnet *s);
int             subnet_num_hosts(const t_subnet *s);      /* hôtes utilisables */
int             subnet_contains(const t_subnet *s, t_ip ip);
int             subnets_overlap(const t_subnet *a, const t_subnet *b);

/* ------------------------------------------------------------------ */
/*  Routage                                                            */
/* ------------------------------------------------------------------ */

typedef struct s_route
{
    t_ip            dest;          /* reseau de destination       */
    uint32_t        dest_mask;     /* masque de destination      */
    t_ip            gateway;       /* passerelle (0 si aucune)   */
}                   t_route;

/* Plus long préfixe correspondant. Retourne l'index de la route, -1 si aucune. */
int             route_match(const t_route *routes, int n, t_ip ip);

/* ------------------------------------------------------------------ */
/*  JSON minimal                                                       */
/* ------------------------------------------------------------------ */

typedef enum
{
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
}                   t_json_type;

typedef struct s_json
{
    t_json_type    type;
    char            *str;            /* JSON_STRING                  */
    double          num;            /* JSON_NUMBER                 */
    int             boolean;        /* JSON_BOOL                   */
    struct s_json    **children;     /* JSON_ARRAY / JSON_OBJECT    */
    char            **keys;         /* JSON_OBJECT                 */
    int             count;
    int             cap;
}                   t_json;

t_json            *json_parse(const char *s);
void            json_free(t_json *j);
t_json            *json_get(const t_json *obj, const char *key);
t_json            *json_at(const t_json *arr, int i);

/* ------------------------------------------------------------------ */
/*  Solution NetPratice (JSON: routes + ifs)                          */
/* ------------------------------------------------------------------ */

typedef struct s_iface
{
    char            *name;
    t_ip            ip;
    int             has_ip;
    uint32_t        mask;
    int             has_mask;
    char            *mask_str;    /* format original (préservé au round-trip) */
}                   t_iface;

typedef struct s_rt_entry
{
    t_ip            dest;
    uint32_t        dest_mask;
    int             has_route;
    t_ip            gateway;
    int             has_gate;
    char            *route_str;    /* chaîne route originale (round-trip) */
}                   t_rt_entry;

typedef struct s_rt_table
{
    char            *name;
    t_rt_entry        *entries;
    int             count;
    int             capacity;
}                   t_rt_table;

typedef struct s_solution
{
    t_iface        *ifs;
    int             n_ifs;
    int             cap_ifs;
    t_rt_table        *rts;
    int             n_rts;
    int             cap_rts;
    int             parse_ok;    /* 0 si un IP/masque/passerelle est invalide */
}                   t_solution;

/* Lecture/écriture */
int            solution_from_json(const t_json *j, t_solution *sol);
int            solution_parse_file(const char *path, t_solution *sol);
int            solution_parse_str(const char *s, t_solution *sol);
char            *solution_to_json(const t_solution *sol);
void            solution_free(t_solution *sol);

/* ------------------------------------------------------------------ */
/*  Définition de niveau (topologie + objectifs)                       */
/* ------------------------------------------------------------------ */

typedef struct s_goal
{
    char            *src;          /* nom du device source       */
    char            *dst;          /* nom du device destination  */
}                   t_goal;

typedef struct s_device_def
{
    char            *name;
    char            **if_names;    /* noms des interfaces        */
    int             n_ifs;
    char            *rt_name;      /* nom de la table de routage */
}                   t_device_def;

typedef struct s_link
{
    char            **ifs;         /* interfaces du sous-réseau    */
    int             n_ifs;
}                   t_link;

typedef struct s_level
{
    char            *name;
    t_device_def        *devices;
    int             n_devices;
    t_goal            *goals;
    int             n_goals;
    t_link            *links;       /* sous-réseaux (topologie)     */
    int             n_links;
}                   t_level;

int            level_parse_file(const char *path, t_level *lvl);
int            level_parse_str(const char *s, t_level *lvl);
void            level_free(t_level *lvl);

/* ------------------------------------------------------------------ */
/*  Validation                                                         */
/* ------------------------------------------------------------------ */

/* Vérifie les masques, passerelles, chevauchements. 0 = ok, -1 = erreur. */
int            solution_validate_config(const t_solution *sol,
                const t_level *lvl, char **err);

/* Simule la connectivité src -> dst. 1 = reachable, 0 = non. */
int            can_reach(const t_solution *sol, const t_level *lvl,
                const char *src_dev, const char *dst_dev);

/* Valide tous les objectifs. 0 = tous OK, -1 = échec. */
int            solution_validate_goals(const t_solution *sol,
                const t_level *lvl, char **err);

/* ------------------------------------------------------------------ */
/*  Solveur                                                            */
/* ------------------------------------------------------------------ */

/* Topologie pour le solveur : sous-réseaux + liens (passerelles). */
typedef struct s_topo_link
{
    int             a;              /* index du sous-réseau A       */
    int             b;              /* index du sous-réseau B       */
    int             router_a_ip;   /* IP du routeur côté A (hôte)*/
    int             router_b_ip;   /* IP du routeur côté B        */
}                   t_topo_link;

typedef struct s_topo
{
    t_subnet        *subnets;
    int             n_subnets;
    t_topo_link        *links;
    int             n_links;
}                   t_topo;

/* Résout un niveau : calcule une solution valide. 0 = ok. */
int            solver_solve(const t_level *lvl, const t_topo *topo,
                t_solution *out);

/* ------------------------------------------------------------------ */
/*  Utilitaires                                                        */
/* ------------------------------------------------------------------ */

void            *xmalloc(size_t size);
void            *xcalloc(size_t count, size_t size);
void            *xrealloc(void *ptr, size_t size);

/* helpers (non présents dans la libft minimale) */
int            ft_snprintf(char *buf, size_t size, const char *fmt, ...);
int            ft_strcmp(const char *s1, const char *s2);
char            *ft_strtok(char *str, const char *delim, char **saveptr);
size_t        ft_strspn(const char *s, const char *accept);
char            *ft_strpbrk(const char *s, const char *accept);
void            ft_free(void *ptr);

# endif
