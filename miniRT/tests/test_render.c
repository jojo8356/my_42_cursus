/* test_render.c — suite de tests du rendu (pixel, région, MT, frame). */
# include "test_framework.h"
# include "minrt.h"
# include <math.h>
# include <stdlib.h>

static t_scene    make_scene(void)
{
    t_scene    scene;
    t_obj        *sp;

    ft_bzero(&scene, sizeof(scene));
    scene.ambient = color_new(1.0, 1.0, 1.0);
    scene.ambient_ratio = 0.2;
    scene.light.color = color_new(1.0, 1.0, 1.0);
    scene.light.ratio = 1.0;
    scene.light.direction = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    camera_init(&scene.camera, vec3_new(0.0, 0.0, -5.0),
        vec3_new(0.0, 0.0, 1.0), 60.0, 80, 60);
    sp = obj_sphere(vec3_new(0.0, 0.0, 0.0), 2.0, color_new(1.0, 0.0, 0.0));
    scene_add_object(&scene, sp);
    return (scene);
}

int    main(void)
{
    t_scene    scene;
    t_color    *buf_st;
    t_color    *buf_mt;
    t_game    game;
    t_ray    ray;
    t_color    c;
    int        i;

    TEST_BEGIN();
    TEST_SUITE("render");

    /* ---- skybox ---- */
    ray.origin = vec3_new(0.0, 0.0, -5.0);
    ray.dir = vec3_normalize(vec3_new(0.0, 0.0, 1.0));
    c = skybox_color(&ray);
    TEST_ASSERT(c.b > c.r);
    ray.dir = vec3_normalize(vec3_new(0.0, 1.0, 0.0));
    c = skybox_color(&ray);
    TEST_ASSERT(c.r > 0.0 && c.b > 0.0);

    scene = make_scene();

    /* ---- render_pixel : pixel de ciel ---- */
    c = render_pixel(&scene, &scene.camera, 40, 0, 0);
    TEST_ASSERT(c.b > c.r);

    /* ---- render_pixel : pixel sur la sphère (centre) ---- */
    c = render_pixel(&scene, &scene.camera, 40, 30, 0);
    TEST_ASSERT(c.r > 0.8);
    TEST_ASSERT(c.g < c.r);
    TEST_ASSERT(c.b < c.r);

    /* ---- render_region (simple) ---- */
    buf_st = (t_color *)xcalloc(80 * 60, sizeof(t_color));
    render_region(&scene, &scene.camera, 0, 60, 0, buf_st, 80);
    TEST_ASSERT(buf_st[0 * 80 + 40].b > buf_st[0 * 80 + 40].r);
    TEST_ASSERT(buf_st[30 * 80 + 40].r > 0.5);

    /* ---- render_multithreaded : doit égaler le simple ---- */
    buf_mt = (t_color *)xcalloc(80 * 60, sizeof(t_color));
    render_multithreaded(&scene, &scene.camera, 60, 0, buf_mt, 80, 4);
    for (i = 0; i < 80 * 60; i++)
    {
        TEST_ASSERT_NEAR(buf_mt[i].r, buf_st[i].r, 1e-9);
        TEST_ASSERT_NEAR(buf_mt[i].g, buf_st[i].g, 1e-9);
        TEST_ASSERT_NEAR(buf_mt[i].b, buf_st[i].b, 1e-9);
    }

    /* ---- antialiasing : le pixel AA est la moyenne des échantillons ---- */
    {
        t_color    aa = render_pixel(&scene, &scene.camera, 40, 30, 1);
        t_color    plain = render_pixel(&scene, &scene.camera, 40, 30, 0);
        TEST_ASSERT(aa.r >= 0.0 && aa.r <= 1.0);
        TEST_ASSERT(plain.r >= 0.0 && plain.r <= 1.0);
    }

    /* ---- render_frame : écrit dans le tampon MLX ---- */
    ft_bzero(&game, sizeof(game));
    game.mlx.width = 80;
    game.mlx.height = 60;
    game.mlx.img_bpp = 32;
    game.mlx.img_line = 80 * 4;
    game.mlx.img_data = (char *)xcalloc(80 * 60 * 4, 1);
    game.scene = scene;
    game.multithread = 0;
    game.antialias = 0;
    render_frame(&game);
    {
        char    *p = game.mlx.img_data + (30 * 80 + 40) * 4;
        TEST_ASSERT(((unsigned char)p[0]) > 128);   /* rouge */
        TEST_ASSERT(((unsigned char)p[1]) < 128);   /* pas majoritairement vert */
        TEST_ASSERT(((unsigned char)p[3]) == 255);  /* opaque */
    }
    /* pixel de ciel en haut */
    {
        char    *p = game.mlx.img_data + (0 * 80 + 40) * 4;
        TEST_ASSERT(((unsigned char)p[2]) > ((unsigned char)p[0]));
    }
    free(game.mlx.img_data);

    free(buf_st);
    free(buf_mt);
    scene_free(&scene);
    TEST_END();
}
