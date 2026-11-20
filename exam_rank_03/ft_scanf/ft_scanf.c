/*
** ft_scanf.c - Exam Rank 03, level 1
** Mimic scanf for the %s, %d and %c conversions.
** Allowed functions: fgetc, ungetc, ferror, feof, isspace, isdigit, stdin,
**                     va_start, va_arg, va_copy, va_end
*/
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

/* Skip any whitespace in the input stream.
** Return -1 on EOF, 0 otherwise (whitespace in format matches 0..n spaces). */
int    match_space(FILE *f)
{
    int    c;

    c = fgetc(f);
    if (c == EOF)
        return (-1);
    while (isspace(c))
        c = fgetc(f);
    if (c != EOF)
        ungetc(c, f);
    return (0);
}

/* Match a single literal character in the input stream.
** Return 1 on match, 0 otherwise (char left in stream on mismatch). */
int    match_char(FILE *f, char c)
{
    int    cur;

    cur = fgetc(f);
    if (cur == EOF)
        return (0);
    if (cur != c)
    {
        ungetc(cur, f);
        return (0);
    }
    return (1);
}

/* %c : read exactly one character (no whitespace skipping). */
int    scan_char(FILE *f, va_list ap)
{
    int    c;
    char    *out;

    out = va_arg(ap, char *);
    c = fgetc(f);
    if (c == EOF)
        return (0);
    *out = (char)c;
    return (1);
}

/* %d : read an optionally signed integer. */
int    scan_int(FILE *f, va_list ap)
{
    int    c;
    int    sign;
    long    val;
    int    *out;

    out = va_arg(ap, int *);
    sign = 1;
    c = fgetc(f);
    if (c == '+' || c == '-')
    {
        if (c == '-')
            sign = -1;
        c = fgetc(f);
    }
    if (!isdigit(c))
    {
        if (c != EOF)
            ungetc(c, f);
        return (0);
    }
    val = 0;
    while (isdigit(c))
    {
        val = val * 10 + (long)(c - '0');
        c = fgetc(f);
    }
    if (c != EOF)
        ungetc(c, f);
    *out = (int)(sign * val);
    return (1);
}

/* %s : read a sequence of non-whitespace characters into a char buffer. */
int    scan_string(FILE *f, va_list ap)
{
    int    c;
    int    i;
    char    *out;

    out = va_arg(ap, char *);
    c = fgetc(f);
    if (c == EOF)
        return (0);
    i = 0;
    while (c != EOF && !isspace(c))
    {
        out[i] = (char)c;
        i++;
        c = fgetc(f);
    }
    if (c != EOF)
        ungetc(c, f);
    out[i] = '\0';
    return (1);
}

int    match_conv(FILE *f, const char **format, va_list ap)
{
    switch (**format)
    {
        case 'c':
            return (scan_char(f, ap));
        case 'd':
            match_space(f);
            return (scan_int(f, ap));
        case 's':
            match_space(f);
            return (scan_string(f, ap));
        case EOF:
            return (-1);
        default:
            return (-1);
    }
}

int    ft_vfscanf(FILE *f, const char *format, va_list ap)
{
    int    nconv = 0;
    int    c;

    c = fgetc(f);
    if (c == EOF)
        return EOF;
    ungetc(c, f);

    while (*format)
    {
        if (*format == '%')
        {
            format++;
            if (match_conv(f, &format, ap) != 1)
                break;
            else
                nconv++;
        }
        else if (isspace(*format))
        {
            if (match_space(f) == -1)
                break;
        }
        else if (match_char(f, *format) != 1)
            break;
        format++;
    }

    if (ferror(f))
        return EOF;
    return (nconv);
}

int    ft_scanf(const char *format, ...)
{
    va_list    ap;
    int        ret;

    va_start(ap, format);
    ret = ft_vfscanf(stdin, format, ap);
    va_end(ap);
    return (ret);
}
