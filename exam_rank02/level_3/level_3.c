/* ************************************************************************** */
/*                                                                          */
/*                                                       :::      ::::::::   */
/*   level_3.c                                          :+:      :+:    :+:   */
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
/* add_prime_sum — sum of prime numbers from args                         */
/* ----------------------------------------------------------------------- */
int add_prime_sum(int ac, char **av)
{
    int i;
    int j;
    int sum;
    int n;

    sum = 0;
    i = 1;
    while (i < ac)
    {
        n = ft_atoi(av[i]);
        if (n <= 0)
        {
            write(1, "Error\n", 6);
            return (-1);
        }
        j = 2;
        while (j <= n)
        {
            int is_prime = 1;
            int k = 2;
            while (k * k <= j)
            {
                if (j % k == 0)
                {
                    is_prime = 0;
                    break;
                }
                k++;
            }
            if (is_prime && j < n)
                sum += j;
            j++;
        }
        i++;
    }
    return (sum);
}

/* ----------------------------------------------------------------------- */
/* epur_str — remove multiple spaces                                     */
/* ----------------------------------------------------------------------- */
char *epur_str(char *str)
{
    int i;
    int j;

    i = 0;
    j = 0;
    while (str[i])
    {
        while (str[i] == ' ')
            i++;
        if (str[i])
        {
            str[j] = str[i];
            j++;
            while (str[i] && str[i] != ' ')
            {
                str[j] = str[i];
                i++;
                j++;
            }
            if (str[i])
                str[j++] = ' ';
        }
    }
    if (j > 0 && str[j - 1] == ' ')
        j--;
    str[j] = 0;
    return (str);
}

/* ----------------------------------------------------------------------- */
/* expand_str — expand digits to repeated chars                            */
/* ----------------------------------------------------------------------- */
char *expand_str(char *str)
{
    int new_len;
    int i;
    int j;
    char *result;

    new_len = 0;
    i = 0;
    while (str[i])
    {
        if (str[i] >= '0' && str[i] <= '9')
            new_len += str[i] - '0' - 1; /* n-1 extra copies */
        else
            new_len++;
        i++;
    }
    result = (char *)malloc(sizeof(char) * (new_len + 1));
    if (!result)
        return (NULL);
    i = 0;
    j = 0;
    while (str[i])
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            int k;
            k = 0;
            while (k < str[i] - '0' - 1)
            {
                result[j++] = str[i - 1];
                k++;
            }
            i++;
        }
        else
        {
            result[j++] = str[i];
            i++;
        }
    }
    result[j] = 0;
    free(str);
    return (result);
}

/* ----------------------------------------------------------------------- */
/* ft_atoi_base — convert string to int in given base                      */
/* ----------------------------------------------------------------------- */
int ft_atoi_base(char *str, char *base)
{
    int i;
    int sign;
    int result;
    int base_len;
    int pos;

    i = 0;
    sign = 1;
    result = 0;
    base_len = ft_strlen(base);
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n'
           || str[i] == '\r' || str[i] == '\f' || str[i] == '\v')
        i++;
    if (str[i] == '-')
    {
        sign = -1;
        i++;
    }
    while (str[i])
    {
        pos = 0;
        while (pos < base_len && str[i] != base[pos])
            pos++;
        if (pos >= base_len)
            break;
        result = result * base_len + pos;
        i++;
    }
    return (result * sign);
}

/* ----------------------------------------------------------------------- */
/* ft_list_size — count elements in linked list                           */
/* ----------------------------------------------------------------------- */
int ft_list_size(list_t *list)
{
    int count;

    count = 0;
    while (list)
    {
        count++;
        list = list->next;
    }
    return (count);
}

/* ----------------------------------------------------------------------- */
/* ft_range — generate array of strings from min to max                    */
/* ----------------------------------------------------------------------- */
char **ft_range(int min, int max)
{
    char **result;
    int i;
    int n;
    int len;
    char *s;

    if (min >= max)
        return (NULL);
    result = (char **)malloc(sizeof(char *) * (max - min + 1));
    if (!result)
        return (NULL);
    i = 0;
    n = min;
    while (n < max)
    {
        int n_abs = n < 0 ? -n : n;
        len = 0;
        int tmp = n_abs;
        if (tmp == 0)
            len = 1;
        while (tmp > 0)
        {
            len++;
            tmp /= 10;
        }
        if (n < 0)
            len++;
        s = (char *)malloc(sizeof(char) * (len + 1));
        if (!s)
            return (NULL);
        int j = len;
        s[j] = 0;
        j--;
        if (n < 0)
        {
            s[j--] = '-';
        }
        tmp = n_abs;
        if (tmp == 0)
            s[j--] = '0';
        while (tmp > 0)
        {
            s[j--] = '0' + (tmp % 10);
            tmp /= 10;
        }
        result[i] = s;
        n++;
        i++;
    }
    result[i] = NULL;
    return (result);
}

/* ----------------------------------------------------------------------- */
/* ft_rrange — generate array of strings from max to min                    */
/* ----------------------------------------------------------------------- */
char **ft_rrange(int min, int max)
{
    char **result;
    int i;
    int n;
    int len;
    char *s;

    if (min >= max)
        return (NULL);
    result = (char **)malloc(sizeof(char *) * (max - min + 1));
    if (!result)
        return (NULL);
    i = 0;
    n = max - 1;
    while (n >= min)
    {
        int n_abs = n < 0 ? -n : n;
        len = 0;
        int tmp = n_abs;
        if (tmp == 0)
            len = 1;
        while (tmp > 0)
        {
            len++;
            tmp /= 10;
        }
        if (n < 0)
            len++;
        s = (char *)malloc(sizeof(char) * (len + 1));
        if (!s)
            return (NULL);
        int j = len;
        s[j] = 0;
        j--;
        if (n < 0)
        {
            s[j--] = '-';
        }
        tmp = n_abs;
        if (tmp == 0)
            s[j--] = '0';
        while (tmp > 0)
        {
            s[j--] = '0' + (tmp % 10);
            tmp /= 10;
        }
        result[i] = s;
        n--;
        i++;
    }
    result[i] = NULL;
    return (result);
}

/* ----------------------------------------------------------------------- */
/* hidenp — hide string s2 in s1                                           */
/* ----------------------------------------------------------------------- */
void hidenp(char *s1, char *s2)
{
    int i;
    int j;

    i = 0;
    j = 0;
    while (s1[i])
    {
        if (s1[i] == s2[j])
            j++;
        write(1, &s1[i], 1);
        i++;
    }
}

/* ----------------------------------------------------------------------- */
/* lcm — least common multiple                                             */
/* ----------------------------------------------------------------------- */
int lcm(int a, int b)
{
    int result;

    if (a == 0 || b == 0)
        return (0);
    result = a;
    while (result % b != 0)
        result += a;
    return (result);
}

/* ----------------------------------------------------------------------- */
/* paramsum — sum of params in string                                      */
/* ----------------------------------------------------------------------- */
int paramsum(char *str)
{
    int i;
    int sum;
    int n;

    sum = 0;
    i = 0;
    while (str[i])
    {
        while (str[i] && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n'))
            i++;
        n = 0;
        while (str[i] >= '0' && str[i] <= '9')
        {
            n = n * 10 + (str[i] - '0');
            i++;
        }
        if (n > 0)
            sum += n;
    }
    return (sum);
}

/* ----------------------------------------------------------------------- */
/* pgcd — greatest common divisor                                         */
/* ----------------------------------------------------------------------- */
int pgcd(int a, int b)
{
    while (b != 0)
    {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return (a);
}

/* ----------------------------------------------------------------------- */
/* print_hex — print number in hexadecimal                                */
/* ----------------------------------------------------------------------- */
void print_hex(unsigned int nb)
{
    char *hex = "0123456789abcdef";

    if (nb >= 16)
        print_hex(nb / 16);
    write(1, &hex[nb % 16], 1);
}

/* ----------------------------------------------------------------------- */
/* rstr_capitalizer — capitalize strings from end                          */
/* ----------------------------------------------------------------------- */
char *rstr_capitalizer(char *str)
{
    int len;
    int i;
    int in_word;

    len = ft_strlen(str) - 1;
    in_word = 0;
    i = len;
    while (i >= 0)
    {
        if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] = str[i] + 32;
        if (str[i] >= 'a' && str[i] <= 'z')
        {
            if (!in_word)
            {
                str[i] = str[i] - 32;
                in_word = 1;
            }
        }
        else
            in_word = 0;
        i--;
    }
    return (str);
}

/* ----------------------------------------------------------------------- */
/* str_capitalizer — capitalize strings                                   */
/* ----------------------------------------------------------------------- */
char *str_capitalizer(char *str)
{
    int i;
    int in_word;

    in_word = 0;
    i = 0;
    while (str[i])
    {
        if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] = str[i] + 32;
        if (str[i] >= 'a' && str[i] <= 'z')
        {
            if (!in_word)
            {
                str[i] = str[i] - 32;
                in_word = 1;
            }
        }
        else
            in_word = 0;
        i++;
    }
    return (str);
}

/* ----------------------------------------------------------------------- */
/* tab_mult — generate multiplication table                               */
/* ----------------------------------------------------------------------- */
char **tab_mult(int nb)
{
    char **result;
    int i;
    int n;
    int len;
    char *s;

    if (nb < 0)
        return (NULL);
    result = (char **)malloc(sizeof(char *) * (nb + 2));
    if (!result)
        return (NULL);
    i = 0;
    n = 0;
    while (i <= nb)
    {
        int val = i * nb;
        int n_abs = val < 0 ? -val : val;
        len = 0;
        int tmp = n_abs;
        if (tmp == 0)
            len = 1;
        while (tmp > 0)
        {
            len++;
            tmp /= 10;
        }
        if (val < 0)
            len++;
        s = (char *)malloc(sizeof(char) * (len + 1));
        if (!s)
            return (NULL);
        int j = len;
        s[j] = 0;
        j--;
        if (val < 0)
        {
            s[j--] = '-';
        }
        tmp = n_abs;
        if (tmp == 0)
            s[j--] = '0';
        while (tmp > 0)
        {
            s[j--] = '0' + (tmp % 10);
            tmp /= 10;
        }
        result[i] = s;
        n++;
        i++;
    }
    result[i] = NULL;
    return (result);
}
