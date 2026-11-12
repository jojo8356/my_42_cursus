/* ************************************************************************** */
/*                                                                          */
/*                                                       :::      ::::::::   */
/*   level_2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 21:00:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 21:00:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# include "exam_rank02.h"
# include <unistd.h>
# include <stdlib.h>

/* ----------------------------------------------------------------------- */
/* alpha_mirror — mirror alphabetical characters                           */
/* ----------------------------------------------------------------------- */
char *alpha_mirror(char *str)
{
    int i;
    int len;

    len = ft_strlen(str);
    i = 0;
    while (i < len)
    {
        if (str[i] >= 'a' && str[i] <= 'z')
            str[i] = 'z' - (str[i] - 'a');
        else if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] = 'Z' - (str[i] - 'A');
        i++;
    }
    return (str);
}

/* ----------------------------------------------------------------------- */
/* camel_to_snake — convert CamelCase to snake_case                        */
/* ----------------------------------------------------------------------- */
char *camel_to_snake(char *str)
{
    int len;
    int i;
    int j;
    char *result;

    len = ft_strlen(str);
    /* Worst case: every char is uppercase (except first) + '_' */
    result = (char *)malloc(sizeof(char) * (len * 2 + 1));
    if (!result)
        return (str);
    i = 0;
    j = 0;
    while (str[i])
    {
        if (str[i] >= 'A' && str[i] <= 'Z')
        {
            if (i != 0)
                result[j++] = '_';
            result[j++] = str[i] + 32;
        }
        else
            result[j++] = str[i];
        i++;
    }
    result[j] = 0;
    /* Always copy back (assumes buffer is large enough) */
    i = 0;
    while (result[i])
    {
        str[i] = result[i];
        i++;
    }
    str[i] = 0;
    free(result);
    return (str);
}

/* ----------------------------------------------------------------------- */
/* do_op — perform arithmetic operation                                    */
/* ----------------------------------------------------------------------- */
int do_op(int a, int b, char op)
{
    if (op == '+')
        return (a + b);
    if (op == '-')
        return (a - b);
    if (op == '*')
        return (a * b);
    if (op == '/')
        return (a / b);
    if (op == '%')
        return (a % b);
    return (0);
}

/* ----------------------------------------------------------------------- */
/* ft_atoi — convert string to int                                         */
/* ----------------------------------------------------------------------- */
int ft_atoi(char *str)
{
    int i;
    int sign;
    int result;

    i = 0;
    sign = 1;
    result = 0;
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n'
           || str[i] == '\r' || str[i] == '\f' || str[i] == '\v')
        i++;
    if (str[i] == '-')
    {
        sign = -1;
        i++;
    }
    else if (str[i] == '+')
        i++;
    while (str[i] >= '0' && str[i] <= '9')
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return (result * sign);
}

/* ----------------------------------------------------------------------- */
/* ft_strcmp — compare two strings                                         */
/* ----------------------------------------------------------------------- */
int ft_strcmp(char *s1, char *s2)
{
    int i;

    i = 0;
    while (s1[i] && s2[i] && s1[i] == s2[i])
        i++;
    return (s1[i] - s2[i]);
}

/* ----------------------------------------------------------------------- */
/* ft_strcspn — find first occurrence of any char from accept              */
/* ----------------------------------------------------------------------- */
int ft_strcspn(char *s, char *accept)
{
    int i;
    int j;

    i = 0;
    while (s[i])
    {
        j = 0;
        while (accept[j])
        {
            if (s[i] == accept[j])
                return (i);
            j++;
        }
        i++;
    }
    return (i);
}

/* ----------------------------------------------------------------------- */
/* ft_strdup — duplicate a string                                          */
/* ----------------------------------------------------------------------- */
char *ft_strdup(char *src)
{
    char *dest;
    int len;
    int i;

    len = ft_strlen(src);
    dest = (char *)malloc(sizeof(char) * (len + 1));
    if (!dest)
        return (NULL);
    i = 0;
    while (src[i])
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
    return (dest);
}

/* ----------------------------------------------------------------------- */
/* ft_strpbrk — find first occurrence of any char from s2 in s1           */
/* ----------------------------------------------------------------------- */
char *ft_strpbrk(char *s1, char *s2)
{
    int i;
    int j;

    i = 0;
    while (s1[i])
    {
        j = 0;
        while (s2[j])
        {
            if (s1[i] == s2[j])
                return (&s1[i]);
            j++;
        }
        i++;
    }
    return (NULL);
}

/* ----------------------------------------------------------------------- */
/* ft_strrev — reverse a string                                            */
/* ----------------------------------------------------------------------- */
char *ft_strrev(char *src)
{
    int len;
    int i;
    char c;

    len = ft_strlen(src) - 1;
    i = 0;
    while (i < len)
    {
        c = src[i];
        src[i] = src[len];
        src[len] = c;
        i++;
        len--;
    }
    return (src);
}

/* ----------------------------------------------------------------------- */
/* ft_strspn — find length of initial segment of s in accept              */
/* ----------------------------------------------------------------------- */
int ft_strspn(char *s, char *accept)
{
    int i;
    int j;
    int found;

    i = 0;
    while (s[i])
    {
        j = 0;
        found = 0;
        while (accept[j])
        {
            if (s[i] == accept[j])
            {
                found = 1;
                break;
            }
            j++;
        }
        if (!found)
            break;
        i++;
    }
    return (i);
}

/* ----------------------------------------------------------------------- */
/* inter — count number of common integers between args                    */
/* ----------------------------------------------------------------------- */
int inter(int ac, char **av)
{
    int i;
    int j;
    int count;
    int n1;
    int n2;

    count = 0;
    if (ac < 2)
        return (0);
    i = 0;
    while (i < ac)
    {
        n1 = ft_atoi(av[i]);
        j = i + 1;
        while (j < ac)
        {
            n2 = ft_atoi(av[j]);
            if (n1 == n2)
                count++;
            j++;
        }
        i++;
    }
    return (count);
}

/* ----------------------------------------------------------------------- */
/* last_word — print the last word of a string                             */
/* ----------------------------------------------------------------------- */
char *last_word(char *str)
{
    int len;
    int end;

    len = ft_strlen(str) - 1;
    while (len >= 0 && (str[len] == ' ' || str[len] == '\t' || str[len] == '\n'))
        len--;
    end = len;
    while (len >= 0 && str[len] != ' ' && str[len] != '\t' && str[len] != '\n')
        len--;
    str[end + 1] = 0;
    return (&str[len + 1]);
}

/* ----------------------------------------------------------------------- */
/* is_power_of_2 — check if n is a power of 2                             */
/* ----------------------------------------------------------------------- */
int is_power_of_2(unsigned int n)
{
    if (n == 0)
        return (0);
    while (n > 1)
    {
        if (n % 2 != 0)
            return (0);
        n /= 2;
    }
    return (1);
}

/* ----------------------------------------------------------------------- */
/* max — find maximum in array                                            */
/* ----------------------------------------------------------------------- */
int max(int tab[], int size)
{
    int i;
    int m;

    if (size == 0)
        return (0);
    m = tab[0];
    i = 1;
    while (i < size)
    {
        if (tab[i] > m)
            m = tab[i];
        i++;
    }
    return (m);
}

/* ----------------------------------------------------------------------- */
/* print_bits — print bits of nb                                          */
/* ----------------------------------------------------------------------- */
void print_bits(unsigned int nb)
{
    int i;

    i = 31;
    while (i >= 0)
    {
        if (nb & (1U << i))
            write(1, "1", 1);
        else
            write(1, "0", 1);
        i--;
    }
    write(1, "\n", 1);
}

/* ----------------------------------------------------------------------- */
/* reverse_bits — reverse bits of nb                                      */
/* ----------------------------------------------------------------------- */
void reverse_bits(unsigned int nb)
{
    unsigned int rev;
    int i;

    rev = 0;
    i = 0;
    while (i < 32)
    {
        rev = (rev << 1) | (nb & 1);
        nb >>= 1;
        i++;
    }
    print_bits(rev);
}

/* ----------------------------------------------------------------------- */
/* snake_to_camel — convert snake_case to CamelCase                       */
/* ----------------------------------------------------------------------- */
char *snake_to_camel(char *str)
{
    int i;
    int j;

    i = 0;
    while (str[i])
    {
        if (str[i] == '_')
        {
            /* Uppercase the next char and shift left */
            str[i] = str[i + 1] - 32;
            j = i + 1;
            while (str[j])
            {
                str[j] = str[j + 1];
                j++;
            }
            /* Don't increment i, process the new char at i */
        }
        else
            i++;
    }
    return (str);
}

/* ----------------------------------------------------------------------- */
/* swap_bits — swap bits at positions 0-1, 2-3, 4-5, 6-7                  */
/* ----------------------------------------------------------------------- */
void swap_bits(void)
{
    unsigned int nb = 0x68;
    unsigned int result;
    int i;

    result = 0;
    i = 0;
    while (i < 32)
    {
        if (i % 4 == 0 || i % 4 == 1)
        {
            if (i % 4 == 0)
                result |= ((nb >> 1) & 1) << i;
            else
                result |= (nb & 1) << i;
        }
        else if (i % 4 == 2)
            result |= ((nb >> 3) & 1) << i;
        else
            result |= ((nb >> 2) & 1) << i;
        nb >>= 2;
        i += 2;
    }
    print_bits(result);
}

/* ----------------------------------------------------------------------- */
/* union_words — concat two strings with space, remove duplicates          */
/* ----------------------------------------------------------------------- */
char *union_words(char *s1, char *s2)
{
    int len1;
    int len2;
    int i;
    int j;
    char *result;
    int pos;

    len1 = ft_strlen(s1);
    len2 = ft_strlen(s2);
    result = (char *)malloc(sizeof(char) * (len1 + len2 + 1));
    if (!result)
        return (NULL);
    i = 0;
    j = 0;
    pos = 0;
    while (s1[i])
    {
        if (s1[i] != ' ')
        {
            int found = 0;
            int k = 0;
            while (k < pos)
            {
                if (result[k] == s1[i])
                    found = 1;
                k++;
            }
            if (!found)
                result[pos++] = s1[i];
        }
        i++;
    }
    while (s2[j])
    {
        if (s2[j] != ' ')
        {
            int found = 0;
            int k = 0;
            while (k < pos)
            {
                if (result[k] == s2[j])
                    found = 1;
                k++;
            }
            if (!found)
                result[pos++] = s2[j];
        }
        j++;
    }
    result[pos] = 0;
    return (result);
}

/* ----------------------------------------------------------------------- */
/* wdmatch — glob-style matching                                           */
/* ----------------------------------------------------------------------- */
int wdmatch(char *s1, char *s2)
{
    while (*s1 && *s2)
    {
        if (*s1 == *s2)
        {
            s1++;
            s2++;
        }
        else if (*s1 == '*')
        {
            if (wdmatch(s1, s2 + 1))
                return (1);
            if (wdmatch(s1 + 1, s2))
                return (1);
            return (0);
        }
        else if (*s2 == '*')
        {
            if (wdmatch(s1 + 1, s2))
                return (1);
            if (wdmatch(s1, s2 + 1))
                return (1);
            return (0);
        }
        else
            return (0);
    }
    if (*s1 == '*' || *s2 == '*')
        return (1);
    return (*s1 == *s2);
}
