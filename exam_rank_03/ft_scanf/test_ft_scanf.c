/*
** test_ft_scanf.c - Compare ft_scanf with the real scanf.
** Usage: ./test_ft_scanf "<format>" < input
** Parses the format, runs both with matching varargs in order, compares.
*/
#include <stdio.h>
#include <string.h>

int    ft_scanf(const char *format, ...);

int    main(int argc, char **argv)
{
    char    format[256];
    char    conv[8];
    int    nconv;
    char    s_r[3][256], s_f[3][256];
    int    d_r[3], d_f[3];
    char    c_r[3], c_f[3];
    int    i;
    int    r_real, r_ft;
    int    equal = 1;

    if (argc != 2) return (1);
    i = 0;
    while (argv[1][i] && i < 255) { format[i] = argv[1][i]; i++; }
    format[i] = 0;

    nconv = 0;
    i = 0;
    while (format[i]) {
        if (format[i] == '%' && format[i+1] &&
            (format[i+1]=='s'||format[i+1]=='d'||format[i+1]=='c'))
            conv[nconv++] = format[i+1];
        i++;
    }
    if (nconv == 0 || nconv > 3) return (1);

    for (i = 0; i < 3; i++) { s_r[i][0] = 0; s_f[i][0] = 0; d_r[i] = 0; d_f[i] = 0; c_r[i] = 0; c_f[i] = 0; }

    /* Real scanf */
    rewind(stdin);
    r_real = -999;
    {
        if (nconv == 1 && conv[0]=='s')
            r_real = scanf(format, s_r[0]);
        if (nconv == 1 && conv[0]=='d')
            r_real = scanf(format, &d_r[0]);
        if (nconv == 1 && conv[0]=='c')
            r_real = scanf(format, &c_r[0]);
        if (nconv == 2 && conv[0]=='s' && conv[1]=='s')
            r_real = scanf(format, s_r[0], s_r[1]);
        if (nconv == 2 && conv[0]=='s' && conv[1]=='d')
            r_real = scanf(format, s_r[0], &d_r[1]);
        if (nconv == 2 && conv[0]=='s' && conv[1]=='c')
            r_real = scanf(format, s_r[0], &c_r[1]);
        if (nconv == 2 && conv[0]=='d' && conv[1]=='s')
            r_real = scanf(format, &d_r[0], s_r[1]);
        if (nconv == 2 && conv[0]=='d' && conv[1]=='d')
            r_real = scanf(format, &d_r[0], &d_r[1]);
        if (nconv == 2 && conv[0]=='d' && conv[1]=='c')
            r_real = scanf(format, &d_r[0], &c_r[1]);
        if (nconv == 2 && conv[0]=='c' && conv[1]=='s')
            r_real = scanf(format, &c_r[0], s_r[1]);
        if (nconv == 2 && conv[0]=='c' && conv[1]=='d')
            r_real = scanf(format, &c_r[0], &d_r[1]);
        if (nconv == 2 && conv[0]=='c' && conv[1]=='c')
            r_real = scanf(format, &c_r[0], &c_r[1]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='s' && conv[2]=='s')
            r_real = scanf(format, s_r[0], s_r[1], s_r[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='s' && conv[2]=='d')
            r_real = scanf(format, s_r[0], s_r[1], &d_r[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='s' && conv[2]=='c')
            r_real = scanf(format, s_r[0], s_r[1], &c_r[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='d' && conv[2]=='s')
            r_real = scanf(format, s_r[0], &d_r[1], s_r[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='d' && conv[2]=='d')
            r_real = scanf(format, s_r[0], &d_r[1], &d_r[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='d' && conv[2]=='c')
            r_real = scanf(format, s_r[0], &d_r[1], &c_r[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='c' && conv[2]=='s')
            r_real = scanf(format, s_r[0], &c_r[1], s_r[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='c' && conv[2]=='d')
            r_real = scanf(format, s_r[0], &c_r[1], &d_r[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='c' && conv[2]=='c')
            r_real = scanf(format, s_r[0], &c_r[1], &c_r[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='s' && conv[2]=='s')
            r_real = scanf(format, &d_r[0], s_r[1], s_r[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='s' && conv[2]=='d')
            r_real = scanf(format, &d_r[0], s_r[1], &d_r[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='s' && conv[2]=='c')
            r_real = scanf(format, &d_r[0], s_r[1], &c_r[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='d' && conv[2]=='s')
            r_real = scanf(format, &d_r[0], &d_r[1], s_r[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='d' && conv[2]=='d')
            r_real = scanf(format, &d_r[0], &d_r[1], &d_r[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='d' && conv[2]=='c')
            r_real = scanf(format, &d_r[0], &d_r[1], &c_r[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='c' && conv[2]=='s')
            r_real = scanf(format, &d_r[0], &c_r[1], s_r[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='c' && conv[2]=='d')
            r_real = scanf(format, &d_r[0], &c_r[1], &d_r[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='c' && conv[2]=='c')
            r_real = scanf(format, &d_r[0], &c_r[1], &c_r[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='s' && conv[2]=='s')
            r_real = scanf(format, &c_r[0], s_r[1], s_r[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='s' && conv[2]=='d')
            r_real = scanf(format, &c_r[0], s_r[1], &d_r[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='s' && conv[2]=='c')
            r_real = scanf(format, &c_r[0], s_r[1], &c_r[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='d' && conv[2]=='s')
            r_real = scanf(format, &c_r[0], &d_r[1], s_r[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='d' && conv[2]=='d')
            r_real = scanf(format, &c_r[0], &d_r[1], &d_r[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='d' && conv[2]=='c')
            r_real = scanf(format, &c_r[0], &d_r[1], &c_r[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='c' && conv[2]=='s')
            r_real = scanf(format, &c_r[0], &c_r[1], s_r[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='c' && conv[2]=='d')
            r_real = scanf(format, &c_r[0], &c_r[1], &d_r[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='c' && conv[2]=='c')
            r_real = scanf(format, &c_r[0], &c_r[1], &c_r[2]);
    }

    /* ft_scanf */
    rewind(stdin);
    r_ft = -999;
    {
        if (nconv == 1 && conv[0]=='s')
            r_ft = ft_scanf(format, s_f[0]);
        if (nconv == 1 && conv[0]=='d')
            r_ft = ft_scanf(format, &d_f[0]);
        if (nconv == 1 && conv[0]=='c')
            r_ft = ft_scanf(format, &c_f[0]);
        if (nconv == 2 && conv[0]=='s' && conv[1]=='s')
            r_ft = ft_scanf(format, s_f[0], s_f[1]);
        if (nconv == 2 && conv[0]=='s' && conv[1]=='d')
            r_ft = ft_scanf(format, s_f[0], &d_f[1]);
        if (nconv == 2 && conv[0]=='s' && conv[1]=='c')
            r_ft = ft_scanf(format, s_f[0], &c_f[1]);
        if (nconv == 2 && conv[0]=='d' && conv[1]=='s')
            r_ft = ft_scanf(format, &d_f[0], s_f[1]);
        if (nconv == 2 && conv[0]=='d' && conv[1]=='d')
            r_ft = ft_scanf(format, &d_f[0], &d_f[1]);
        if (nconv == 2 && conv[0]=='d' && conv[1]=='c')
            r_ft = ft_scanf(format, &d_f[0], &c_f[1]);
        if (nconv == 2 && conv[0]=='c' && conv[1]=='s')
            r_ft = ft_scanf(format, &c_f[0], s_f[1]);
        if (nconv == 2 && conv[0]=='c' && conv[1]=='d')
            r_ft = ft_scanf(format, &c_f[0], &d_f[1]);
        if (nconv == 2 && conv[0]=='c' && conv[1]=='c')
            r_ft = ft_scanf(format, &c_f[0], &c_f[1]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='s' && conv[2]=='s')
            r_ft = ft_scanf(format, s_f[0], s_f[1], s_f[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='s' && conv[2]=='d')
            r_ft = ft_scanf(format, s_f[0], s_f[1], &d_f[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='s' && conv[2]=='c')
            r_ft = ft_scanf(format, s_f[0], s_f[1], &c_f[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='d' && conv[2]=='s')
            r_ft = ft_scanf(format, s_f[0], &d_f[1], s_f[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='d' && conv[2]=='d')
            r_ft = ft_scanf(format, s_f[0], &d_f[1], &d_f[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='d' && conv[2]=='c')
            r_ft = ft_scanf(format, s_f[0], &d_f[1], &c_f[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='c' && conv[2]=='s')
            r_ft = ft_scanf(format, s_f[0], &c_f[1], s_f[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='c' && conv[2]=='d')
            r_ft = ft_scanf(format, s_f[0], &c_f[1], &d_f[2]);
        if (nconv == 3 && conv[0]=='s' && conv[1]=='c' && conv[2]=='c')
            r_ft = ft_scanf(format, s_f[0], &c_f[1], &c_f[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='s' && conv[2]=='s')
            r_ft = ft_scanf(format, &d_f[0], s_f[1], s_f[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='s' && conv[2]=='d')
            r_ft = ft_scanf(format, &d_f[0], s_f[1], &d_f[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='s' && conv[2]=='c')
            r_ft = ft_scanf(format, &d_f[0], s_f[1], &c_f[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='d' && conv[2]=='s')
            r_ft = ft_scanf(format, &d_f[0], &d_f[1], s_f[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='d' && conv[2]=='d')
            r_ft = ft_scanf(format, &d_f[0], &d_f[1], &d_f[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='d' && conv[2]=='c')
            r_ft = ft_scanf(format, &d_f[0], &d_f[1], &c_f[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='c' && conv[2]=='s')
            r_ft = ft_scanf(format, &d_f[0], &c_f[1], s_f[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='c' && conv[2]=='d')
            r_ft = ft_scanf(format, &d_f[0], &c_f[1], &d_f[2]);
        if (nconv == 3 && conv[0]=='d' && conv[1]=='c' && conv[2]=='c')
            r_ft = ft_scanf(format, &d_f[0], &c_f[1], &c_f[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='s' && conv[2]=='s')
            r_ft = ft_scanf(format, &c_f[0], s_f[1], s_f[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='s' && conv[2]=='d')
            r_ft = ft_scanf(format, &c_f[0], s_f[1], &d_f[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='s' && conv[2]=='c')
            r_ft = ft_scanf(format, &c_f[0], s_f[1], &c_f[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='d' && conv[2]=='s')
            r_ft = ft_scanf(format, &c_f[0], &d_f[1], s_f[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='d' && conv[2]=='d')
            r_ft = ft_scanf(format, &c_f[0], &d_f[1], &d_f[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='d' && conv[2]=='c')
            r_ft = ft_scanf(format, &c_f[0], &d_f[1], &c_f[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='c' && conv[2]=='s')
            r_ft = ft_scanf(format, &c_f[0], &c_f[1], s_f[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='c' && conv[2]=='d')
            r_ft = ft_scanf(format, &c_f[0], &c_f[1], &d_f[2]);
        if (nconv == 3 && conv[0]=='c' && conv[1]=='c' && conv[2]=='c')
            r_ft = ft_scanf(format, &c_f[0], &c_f[1], &c_f[2]);
    }

    if (r_real != r_ft) equal = 0;
    for (i = 0; i < nconv; i++)
    {
        if (conv[i] == 's' && strcmp(s_r[i], s_f[i]) != 0) equal = 0;
        if (conv[i] == 'd' && d_r[i] != d_f[i]) equal = 0;
        if (conv[i] == 'c' && c_r[i] != c_f[i]) equal = 0;
    }

    printf("REAL ret=%d", r_real);
    for (i = 0; i < nconv; i++) {
        if (conv[i] == 's') printf(" [%s]", s_r[i]);
        else if (conv[i] == 'd') printf(" %d", d_r[i]);
        else printf(" %c", c_r[i]);
    }
    printf("\nFT   ret=%d", r_ft);
    for (i = 0; i < nconv; i++) {
        if (conv[i] == 's') printf(" [%s]", s_f[i]);
        else if (conv[i] == 'd') printf(" %d", d_f[i]);
        else printf(" %c", c_f[i]);
    }
    printf("\n");
    printf(equal ? "MATCH\n" : "MISMATCH\n");
    return (equal ? 0 : 1);
}
