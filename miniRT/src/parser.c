/* ************************************************************************** */
/*                                                                            */
/*   parser.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Lecteur de scènes .rt (A/C/L + sp/pl/cy/co/cu + tags bonus).          */
/* ************************************************************************** */

# include "minrt.h"
# include <math.h>

/* Libère un tableau de chaînes produit par ft_split. */
static void    free_split(char **toks)
{
    int    i;

    if (!toks)
        return;
    i = 0;
    while (toks[i])
    {
        free(toks[i]);
        i++;
    }
    free(toks);
}

/* Lit tout le fichier dans un tampon alloué. */
static char    *read_all(FILE *f)
{
    size_t    cap;
    size_t    len;
    char    *buf;

    cap = 4096;
    buf = (char *)xmalloc(cap);
    buf[0] = '\0';
    len = 0;
    while (1)
    {
        size_t    n;

        if (len + 1024 + 1 > cap)
        {
            cap *= 2;
            buf = (char *)xrealloc(buf, cap);
        }
        n = fread(buf + len, 1, 1024, f);
        len += n;
        buf[len] = '\0';
        if (n < 1024)
            break;
    }
    return (buf);
}

/* ---------- helpers de parsing ---------- */

int    parse_double(const char *s, double *out)
{
    char    *end;

    *out = strtod(s, &end);
    while (*end == ' ' || *end == '\t')
        end++;
    return (*end == '\0' ? 0 : -1);
}

int    parse_vec3(const char *s, t_vec3 *out)
{
    char    **parts;
    double    x;
    double    y;
    double    z;
    int        n;

    parts = ft_split(s, ',');
    if (!parts)
        return (-1);
    n = 0;
    while (parts[n])
        n++;
    if (n != 3)
    {
        free_split(parts);
        return (-1);
    }
    if (parse_double(parts[0], &x) || parse_double(parts[1], &y)
        || parse_double(parts[2], &z))
    {
        free_split(parts);
        return (-1);
    }
    free_split(parts);
    *out = vec3_new(x, y, z);
    return (0);
}

int    parse_color(const char *s, t_color *out)
{
    char    **parts;
    double    r;
    double    g;
    double    b;
    double    mx;
    int        n;

    parts = ft_split(s, ',');
    if (!parts)
        return (-1);
    n = 0;
    while (parts[n])
        n++;
    if (n != 3)
    {
        free_split(parts);
        return (-1);
    }
    if (parse_double(parts[0], &r) || parse_double(parts[1], &g)
        || parse_double(parts[2], &b))
    {
        free_split(parts);
        return (-1);
    }
    free_split(parts);
    mx = fmax(fmax(r, g), b);
    if (mx > 1.0)
    {
        r /= 255.0;
        g /= 255.0;
        b /= 255.0;
    }
    *out = color_new(r, g, b);
    return (0);
}

/* ---------- gestion de la liste d'objets ---------- */

void    scene_add_object(t_scene *scene, t_obj *obj)
{
    if (scene->obj_count >= scene->obj_capacity)
    {
        size_t    new_cap;

        new_cap = scene->obj_capacity ? scene->obj_capacity * 2 : 4;
        scene->objects = xrealloc(scene->objects,
            new_cap * sizeof(t_obj));
        scene->obj_capacity = new_cap;
    }
    scene->objects[scene->obj_count++] = *obj;
    free(obj);
}

/* ---------- application des tags bonus ---------- */

static int    pattern_from_name(const char *name)
{
    if (ft_strncmp(name, "checker", 7) == 0)
        return (PAT_CHECKER);
    if (ft_strncmp(name, "stripe", 6) == 0)
        return (PAT_STRIPE);
    if (ft_strncmp(name, "gradient", 8) == 0)
        return (PAT_GRADIENT);
    if (ft_strncmp(name, "ring", 4) == 0)
        return (PAT_RING);
    if (ft_strncmp(name, "test", 4) == 0)
        return (PAT_TEST);
    return (PAT_NONE);
}

static void    apply_tag(t_obj *obj, const char *key, const char *val)
{
    if (ft_strncmp(key, "re", 2) == 0)
        obj->mat.reflection = atof(val);
    else if (ft_strncmp(key, "tr", 2) == 0)
        obj->mat.transparency = atof(val);
    else if (ft_strncmp(key, "ri", 2) == 0)
        obj->mat.refr_index = atof(val);
    else if (ft_strncmp(key, "sp", 2) == 0)
        obj->mat.specular = atof(val);
    else if (ft_strncmp(key, "sh", 2) == 0)
        obj->mat.shininess = atof(val);
    else if (ft_strncmp(key, "texture", 7) == 0)
    {
        free(obj->mat.texture);
        obj->mat.texture = ft_strdup(val);
        obj->mat.tex = (t_texture *)xcalloc(1, sizeof(t_texture));
        if (texture_load_by_name(obj->mat.tex, val) < 0)
        {
            free(obj->mat.tex);
            obj->mat.tex = NULL;
        }
    }
    else if (ft_strncmp(key, "pattern", 7) == 0)
        obj->mat.pattern = pattern_from_name(val);
    else if (ft_strncmp(key, "pc1", 3) == 0)
        parse_color(val, &obj->mat.pattern_c1);
    else if (ft_strncmp(key, "pc2", 3) == 0)
        parse_color(val, &obj->mat.pattern_c2);
}

/* Applique les tags bonus restants (tokens contenant ':'). */
static void    apply_bonus_tags(t_obj *obj, char **toks, int from)
{
    int    i;

    i = from;
    while (toks[i])
    {
        char    *colon;

        if ((colon = ft_strchr(toks[i], ':')))
        {
            *colon = '\0';
            apply_tag(obj, toks[i], colon + 1);
        }
        i++;
    }
}

/* ---------- parsing des entités ---------- */

static int    parse_ambient(t_scene *scene, char **toks)
{
    double    ratio;
    t_color    color;

    if (!toks[1] || !toks[2])
        return (-1);
    if (parse_double(toks[1], &ratio) || parse_color(toks[2], &color))
        return (-1);
    scene->ambient_ratio = ratio;
    scene->ambient = color;
    return (0);
}

static int    parse_camera(t_scene *scene, char **toks)
{
    t_vec3    pos;
    t_vec3    orient;
    double    fov;

    if (!toks[1] || !toks[2] || !toks[3])
        return (-1);
    if (parse_vec3(toks[1], &pos) || parse_vec3(toks[2], &orient)
        || parse_double(toks[3], &fov))
        return (-1);
    camera_init(&scene->camera, pos, orient, fov,
        scene->camera.width ? scene->camera.width : WINDOW_W,
        scene->camera.height ? scene->camera.height : WINDOW_H);
    return (0);
}

static int    parse_light(t_scene *scene, char **toks)
{
    t_vec3    pos;
    double    ratio;
    t_color    color;

    if (!toks[1] || !toks[2] || !toks[3])
        return (-1);
    if (parse_vec3(toks[1], &pos) || parse_double(toks[2], &ratio)
        || parse_color(toks[3], &color))
        return (-1);
    scene->light.position = pos;
    scene->light.ratio = ratio;
    scene->light.color = color;
    scene->light.direction = vec3_normalize(vec3_neg(pos));
    if (vec3_length_sq(scene->light.direction) < EPS)
        scene->light.direction = vec3_new(0.0, -1.0, 0.0);
    return (0);
}

static int    parse_object(t_scene *scene, char **toks)
{
    const char    *cmd = toks[0];
    t_obj        *obj;
    t_vec3        p;
    t_vec3        orient;
    t_color        color;
    double        d1;
    double        d2;
    int            n;

    if (ft_strncmp(cmd, "sp", 2) == 0)
    {
        if (!toks[1] || !toks[2] || !toks[3])
            return (-1);
        if (parse_vec3(toks[1], &p) || parse_double(toks[2], &d1)
            || parse_color(toks[3], &color))
            return (-1);
        obj = obj_sphere(p, d1, color);
        apply_bonus_tags(obj, toks, 4);
    }
    else if (ft_strncmp(cmd, "pl", 2) == 0)
    {
        if (!toks[1] || !toks[2] || !toks[3])
            return (-1);
        if (parse_vec3(toks[1], &p) || parse_vec3(toks[2], &orient)
            || parse_color(toks[3], &color))
            return (-1);
        obj = obj_plane(p, orient, color);
        apply_bonus_tags(obj, toks, 4);
    }
    else if (ft_strncmp(cmd, "cy", 2) == 0)
    {
        if (!toks[1] || !toks[2] || !toks[3] || !toks[4] || !toks[5])
            return (-1);
        if (parse_vec3(toks[1], &p) || parse_vec3(toks[2], &orient)
            || parse_double(toks[3], &d1) || parse_double(toks[4], &d2)
            || parse_color(toks[5], &color))
            return (-1);
        obj = obj_cylinder(p, orient, d1, d2, color);
        apply_bonus_tags(obj, toks, 6);
    }
    else if (ft_strncmp(cmd, "co", 2) == 0)
    {
        if (!toks[1] || !toks[2] || !toks[3] || !toks[4] || !toks[5])
            return (-1);
        if (parse_vec3(toks[1], &p) || parse_vec3(toks[2], &orient)
            || parse_double(toks[3], &d1) || parse_double(toks[4], &d2)
            || parse_color(toks[5], &color))
            return (-1);
        {
            double    half = atan((d1 / 2.0) / d2) * 180.0 / M_PI;
            obj = obj_cone(p, orient, half, d2, color);
        }
        apply_bonus_tags(obj, toks, 6);
    }
    else if (ft_strncmp(cmd, "cu", 2) == 0)
    {
        if (!toks[1] || !toks[2] || !toks[3] || !toks[4])
            return (-1);
        if (parse_vec3(toks[1], &p) || parse_vec3(toks[2], &orient)
            || parse_double(toks[3], &d1) || parse_color(toks[4], &color))
            return (-1);
        obj = obj_cube_at(p, orient, d1, color);
        apply_bonus_tags(obj, toks, 5);
    }
    else
        return (-1);
    n = 0;
    while (toks[n])
        n++;
    scene_add_object(scene, obj);
    return (0);
}

/* ---------- boucle principale ---------- */

static int    parse_line(t_scene *scene, const char *line)
{
    char    **toks;
    int        ret;

    if (line[0] == '#' || line[0] == ' ' || line[0] == '\t'
        || line[0] == '\r')
        return (0);
    toks = ft_split(line, ' ');
    if (!toks || !toks[0])
    {
        if (toks)
            free_split(toks);
        return (0);
    }
    ret = -1;
    if (ft_strncmp(toks[0], "A", 1) == 0)
        ret = parse_ambient(scene, toks);
    else if (ft_strncmp(toks[0], "C", 1) == 0)
        ret = parse_camera(scene, toks);
    else if (ft_strncmp(toks[0], "L", 1) == 0)
        ret = parse_light(scene, toks);
    else
        ret = parse_object(scene, toks);
    free_split(toks);
    return (ret);
}

int    parse_rt_file(FILE *f, t_scene *scene)
{
    char    *content;
    char    *line;

    content = read_all(f);
    if (!content)
        return (-1);
    line = content;
    while (line && *line)
    {
        char    *nl;

        nl = ft_strchr(line, '\n');
        if (nl)
            *nl = '\0';
        if (parse_line(scene, line) < 0)
        {
            free(content);
            return (-1);
        }
        if (!nl)
            break;
        line = nl + 1;
    }
    free(content);
    return (0);
}

int    parse_rt(const char *path, t_scene *scene)
{
    FILE    *f;
    int        ret;

    f = fopen(path, "r");
    if (!f)
        return (-1);
    ret = parse_rt_file(f, scene);
    fclose(f);
    return (ret);
}

/* ---------- validation / libération ---------- */

int    scene_validate(const t_scene *scene)
{
    if (scene->obj_count == 0)
        return (-1);
    if (vec3_length_sq(scene->camera.direction) < EPS)
        return (-1);
    return (0);
}

void    scene_free(t_scene *scene)
{
    free(scene->objects);
    scene->objects = NULL;
    scene->obj_count = 0;
    scene->obj_capacity = 0;
}
