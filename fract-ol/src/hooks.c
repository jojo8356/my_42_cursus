/*
** hooks.c — miniLibX event handlers (keyboard, mouse, expose, close).
**
** All handlers receive the t_fractal* as their `param` argument (the Norm
** forbids global variables and caps functions at 4 parameters).
**
** Controls:
**   ▲ ▼ ◄ ►        move view
**   Mouse wheel     zoom in/out (follows cursor)
**   + / -           zoom in/out (centered)
**   h / c           cycle color palette
**   g               toggle grayscale
**   q / w           decrease / increase max iterations
**   r               reset view
**   ESC / close     quit
*/

# include "fract-ol.h"
# include <stdlib.h>

/* Movement step in complex-plane units per key press. */
# define MOVE_STEP 0.3
/* Zoom factor multiplier for wheel / +/-  presses. */
# define ZOOM_IN  0.8
# define ZOOM_OUT 1.25

/* ----------------------------------------------------------------------- */
int on_close(void *param)
{
    (void)param;
    return 0;   /* return 1 would force-quit; returning 0 lets the loop end */
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
    t_fractal *f = (t_fractal *)param;

    switch (keycode)
    {
        /* ---- movement ---- */
        case 65361: /* Left  */ shift_view(f, -MOVE_STEP, 0); break;
        case 65362: /* Up    */ shift_view(f, 0, MOVE_STEP);  break;
        case 65363: /* Right */ shift_view(f,  MOVE_STEP, 0); break;
        case 65364: /* Down  */ shift_view(f, 0, -MOVE_STEP); break;

        /* ---- zoom (centered) ---- */
        case 43:   /* '+' */ case 61: { zoom_at(f, WIDTH / 2, HEIGHT / 2, ZOOM_IN); break; }
        case 45:   /* '-' */ case 45: { zoom_at(f, WIDTH / 2, HEIGHT / 2, ZOOM_OUT); break; }

        /* ---- color palette cycle ---- */
        case 104:  /* 'h' */ change_palette(f); break;
        case 99:   /* 'c' */ change_palette(f); break;

        /* ---- grayscale toggle ---- */
        case 103:  /* 'g' */ toggle_grayscale(f); break;

        /* ---- iteration count ---- */
        case 113:  /* 'q' */ if (f->max_iter > 50)  f->max_iter -= 50; break;
        case 119:  /* 'w' */ f->max_iter += 50; break;

        /* ---- reset view ---- */
        case 114:  /* 'r' */ init_viewport(f); break;

        /* ---- quit ---- */
        case 65305: /* ESC */ return cleanup_and_exit(f, 0);
    }
    return redraw(f);
}

/* ----------------------------------------------------------------------- */
int on_mouse_press(int button, int x, int y, void *param)
{
    (void)x;
    (void)y;
    t_fractal *f = (t_fractal *)param;

    /* Mouse-wheel zoom (button 4 = scroll up, 5 = scroll down). */
    if (button == 4)
        zoom_at(f, x, y, ZOOM_IN);
    else if (button == 5)
        zoom_at(f, x, y, ZOOM_OUT);

    return redraw(f);
}

/* ----------------------------------------------------------------------- */
int on_mouse_move(int x, int y, void *param)
{
    (void)x;
    (void)y;
    /* Reserved for future mouse-driven Julia parameter updates. */
    return 0;
}

/* ----------------------------------------------------------------------- */
void change_palette(t_fractal *f)
{
    f->palette = (f->palette + 1) % PALETTE_COUNT;
}

/* ----------------------------------------------------------------------- */
void toggle_grayscale(t_fractal *f)
{
    f->grayscale = !f->grayscale;
}
