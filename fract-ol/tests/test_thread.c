/*
** test_thread.c — Tests for multithreaded rendering determinism.
**
** Verifies:
**   •  Multi-threaded render produces same result as single-threaded
**   •  Thread count is reasonable (1–MAX_THREADS)
**   •  No race conditions in pixel buffer (each row written once)
**   •  Threading overhead doesn't corrupt the image
*/



# include "fract-ol.h"
# include "test_framework.h"

# include <pthread.h>
# include <string.h>
# include <math.h>

/* Render a range of rows into a shared buffer. */
typedef struct s_test_render {
    t_fractal *f;
    int y_start;
    int y_end;
    char *buffer;
} t_test_render;

/* ═══════════════════════════════════════════════════════════ */
void *test_render_range(void *arg)
{
    t_test_render *a = (t_test_render *)arg;
    /* Local copy to avoid race on z_r, z_i, c_r, c_i. */
    t_fractal local_f = *a->f;
    local_f.img_data = a->buffer;
    for (int y = a->y_start; y < a->y_end; y++)
    {
        for (int x = 0; x < WIDTH; x++)
            compute_pixel(&local_f, x, y);
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════ */
int main(void)
{
    TEST_BEGIN();

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Thread count
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Thread count");

    int n = get_max_threads();
    TEST_ASSERT(n >= 1);
    TEST_ASSERT(n <= MAX_THREADS);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Multithreaded determinism
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Multithreaded determinism");

    /* Render the same image single-threaded vs multi-threaded. */
    static char buffer_single[WIDTH * HEIGHT * 4];
    static char buffer_multi[WIDTH * HEIGHT * 4];

    t_fractal f;
    test_init_fractal(&f, FRACTAL_JULIA, -0.7, 0.27015);

    /* ---- single-threaded render ---- */
    memset(buffer_single, 0, sizeof(buffer_single));
    f.img_data = buffer_single;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;
    for (int y = 0; y < HEIGHT; y++)
        render_row(&f, y);

    /* ---- multi-threaded render ---- */
    memset(buffer_multi, 0, sizeof(buffer_multi));
    f.img_data = buffer_multi;
    f.bits_per_pixel = 32;
    f.line_size = WIDTH * 4;

    int threads = get_max_threads();
    int rows_per_thread = HEIGHT / threads;
    pthread_t tid[MAX_THREADS];
    t_test_render args[MAX_THREADS];

    for (int t = 0; t < threads; t++)
    {
        args[t].f = &f;
        args[t].y_start = t * rows_per_thread;
        args[t].y_end = (t == threads - 1) ? HEIGHT : (t + 1) * rows_per_thread;
        args[t].buffer = buffer_multi;
        pthread_create(&tid[t], NULL, test_render_range, &args[t]);
    }
    for (int t = 0; t < threads; t++)
        pthread_join(tid[t], NULL);

    /* ---- compare ---- */
    TEST_ASSERT(memcmp(buffer_single, buffer_multi, sizeof(buffer_single)) == 0);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Multi-fractal threading
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Multi-fractal threading");

    /* Test threading with different fractal types. */
    int types[] = { FRACTAL_MANDELBROT, FRACTAL_BURNING_SHIP, FRACTAL_TRICORN, FRACTAL_MULTIBROT };
    int c_r[] = { 0, 0, 0, 0 };
    int c_i[] = { 0, 0, 0, 0 };

    for (int i = 0; i < 4; i++)
    {
        memset(buffer_multi, 0, sizeof(buffer_multi));
        memset(buffer_single, 0, sizeof(buffer_single));

        test_init_fractal(&f, types[i], c_r[i], c_i[i]);
        f.img_data = buffer_single;
        f.bits_per_pixel = 32;
        f.line_size = WIDTH * 4;
        for (int y = 0; y < 20; y++)   /* small sample */
            render_row(&f, y);

        test_init_fractal(&f, types[i], c_r[i], c_i[i]);
        f.img_data = buffer_multi;
        f.bits_per_pixel = 32;
        f.line_size = WIDTH * 4;

        threads = get_max_threads();
        rows_per_thread = 20 / threads;
        if (rows_per_thread == 0) rows_per_thread = 1;

        for (int t = 0; t < threads; t++)
        {
            args[t].f = &f;
            args[t].y_start = t * rows_per_thread;
            args[t].y_end = (t == threads - 1) ? 20 : (t + 1) * rows_per_thread;
            args[t].buffer = buffer_multi;
            pthread_create(&tid[t], NULL, test_render_range, &args[t]);
        }
        for (int t = 0; t < threads; t++)
            pthread_join(tid[t], NULL);

        /* Compare first 20 rows. */
        TEST_ASSERT(memcmp(buffer_single, buffer_multi, 20 * WIDTH * 4 * 4) == 0);
    }

    return TEST_END();
}
