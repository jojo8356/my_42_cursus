/*
** hooks.c — MiniLibX event handlers (keyboard, mouse, expose, close).
**
** Controls:
**   W A S D        translate view
**   Arrow keys     rotate around X/Y
**   Scroll wheel   zoom in/out
**   C              cycle color mode
**   R              reset view
**   ESC / close    quit
*/

# include "fdf.h"
# include <stdlib.h>

/* Movement step. */
# define MOVE_STEP 10.0
# define ROT_STEP  0.05
# define ZOOM_IN   0.9
# define ZOOM_OUT  1.1

/* ----------------------------------------------------------------------- */
int on_close(void *param)
{
    (void)param;
    return 0;
}

/* ----------------------------------------------------------------------- */
int on_expose(void *param)
{
    (void)param;
    return 0;
}

/* ----------------------------------------------------------------------- */
int on_key_press(int keycode, void *param)
{
    t_env *env = (t_env *)param;

    switch (keycode)
    {
        /* ---- translation ---- */
        case 119: /* W */ translate(env, 0, -MOVE_STEP); break;
        case 115: /* S */ translate(env, 0,  MOVE_STEP); break;
        case 97:  /* A */ translate(env, -MOVE_STEP, 0); break;
        case 100: /* D */ translate(env,  MOVE_STEP, 0); break;

        /* ---- rotation ---- */
        case 65363: /* Right  */ rotate(env, 'y',  ROT_STEP); break;
        case 65364: /* Down   */ rotate(env, 'x', -ROT_STEP); break;
        case 65361: /* Left   */ rotate(env, 'y', -ROT_STEP); break;
        case 65362: /* Up     */ rotate(env, 'x',  ROT_STEP); break;

        /* ---- color cycling ---- */
        case 99: /* C */ cycle_color_mode(env); break;

        /* ---- reset ---- */
        case 114: /* R */
            env->scale = 1.0;
            env->offset_x = 0.0;
            env->offset_y = 0.0;
            env->rot_x = 0.0;
            env->rot_y = 0.0;
            env->rot_z = 0.0;
            env->iso_x = ISO_ANGLE_X;
            env->iso_y = ISO_ANGLE_Y;
            break;

        /* ---- quit ---- */
        case 65305: /* ESC */
            cleanup(env);
            exit(0);
    }
    return redraw(env);
}

/* ----------------------------------------------------------------------- */
int on_mouse_press(int button, int x, int y, void *param)
{
    (void)x;
    (void)y;
    t_env *env = (t_env *)param;

    if (button == 4)      /* scroll up → zoom in */
        zoom(env, ZOOM_IN);
    else if (button == 5) /* scroll down → zoom out */
        zoom(env, ZOOM_OUT);

    return redraw(env);
}

/* ----------------------------------------------------------------------- */
int on_mouse_move(int x, int y, void *param)
{
    (void)x;
    (void)y;
    (void)param;
    return 0;
}

/* ----------------------------------------------------------------------- */
void zoom(t_env *env, double factor)
{
    env->scale *= factor;
    if (env->scale < 0.1)
        env->scale = 0.1;
    if (env->scale > 10.0)
        env->scale = 10.0;
}

/* ----------------------------------------------------------------------- */
void translate(t_env *env, double dx, double dy)
{
    env->offset_x += dx;
    env->offset_y += dy;
}

/* ----------------------------------------------------------------------- */
void rotate(t_env *env, char axis, double direction)
{
    switch (axis)
    {
        case 'x': env->rot_x += direction; break;
        case 'y': env->rot_y += direction; break;
        case 'z': env->rot_z += direction; break;
    }
}
