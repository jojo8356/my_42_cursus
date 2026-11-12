/* ************************************************************************** */
/*                                                                          */
/*                                                       :::      ::::::::   */
/*   level_4.c                                          :+:      :+:    :+:   */
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
# include <string.h>

/* ----------------------------------------------------------------------- */
/* flood_fill — fill connected region in a 2D map                         */
/* ----------------------------------------------------------------------- */
void flood_fill(int y, int x, int value, int **map, int height, int width)
{
    if (y < 0 || y >= height || x < 0 || x >= width)
        return ;
    if (map[y][x] != 0)
        return ;
    map[y][x] = value;
    flood_fill(y - 1, x, value, map, height, width);
    flood_fill(y + 1, x, value, map, height, width);
    flood_fill(y, x - 1, value, map, height, width);
    flood_fill(y, x + 1, value, map, height, width);
}

/* ----------------------------------------------------------------------- */
/* fprime — return prime factorization as string                          */
/* ----------------------------------------------------------------------- */
char *fprime(int nb)
{
    char *result;
    int len;
    int pos;
    int i;
    int original;

    if (nb <= 1)
    {
        result = (char *)malloc(sizeof(char) * 2);
        if (!result)
            return (NULL);
        result[0] = (nb == 1) ? '1' : '0';
        result[1] = 0;
        return (result);
    }
    original = nb;
    len = 0;
    i = 2;
    while (i * i <= original)
    {
        while (original % i == 0)
        {
            char *s = ft_itoa(i);
            len += ft_strlen(s);
            free(s);
            original /= i;
        }
        i++;
    }
    if (original > 1)
    {
        char *s = ft_itoa(original);
        len += ft_strlen(s);
        free(s);
    }
    result = (char *)malloc(sizeof(char) * (len + 1));
    if (!result)
        return (NULL);
    pos = 0;
    original = nb;
    i = 2;
    while (i * i <= original)
    {
        while (original % i == 0)
        {
            char *s = ft_itoa(i);
            int j = 0;
            while (s[j])
                result[pos++] = s[j++];
            free(s);
            original /= i;
        }
        i++;
    }
    if (original > 1)
    {
        char *s = ft_itoa(original);
        int j = 0;
        while (s[j])
            result[pos++] = s[j++];
        free(s);
    }
    result[pos] = 0;
    return (result);
}

/* ----------------------------------------------------------------------- */
/* ft_itoa — convert int to string                                        */
/* ----------------------------------------------------------------------- */
char *ft_itoa(int n)
{
    char *result;
    int len;
    int i;
    int sign;
    int tmp;

    len = 0;
    sign = 1;
    tmp = n;
    if (n < 0)
    {
        sign = -1;
        tmp = -n;
    }
    if (tmp == 0)
        len = 1;
    while (tmp > 0)
    {
        len++;
        tmp /= 10;
    }
    result = (char *)malloc(sizeof(char) * (len + 1));
    if (!result)
        return (NULL);
    i = len;
    result[i] = 0;
    i--;
    if (sign == -1)
        result[i--] = '-';
    if (n == 0)
        result[i--] = '0';
    tmp = n < 0 ? -n : n;
    while (tmp > 0)
    {
        result[i--] = '0' + (tmp % 10);
        tmp /= 10;
    }
    return (result);
}

/* ----------------------------------------------------------------------- */
/* ft_list_foreach — apply function to each element                        */
/* ----------------------------------------------------------------------- */
void ft_list_foreach(list_t *list, void (*f)(void *))
{
    while (list)
    {
        f(list->content);
        list = list->next;
    }
}

/* ----------------------------------------------------------------------- */
/* ft_list_remove_if — remove elements matching condition                  */
/* ----------------------------------------------------------------------- */
void ft_list_remove_if(list_t *list, int cmp(void *, void *), void (*f)(void *))
{
    list_t *node;
    list_t *prev;

    node = list;
    prev = NULL;
    while (node)
    {
        if (cmp(node->content, NULL) == 1)
        {
            list_t *next = node->next;
            if (prev)
                prev->next = next;
            if (f)
                f(node->content);
            free(node);
            node = next;
        }
        else
        {
            prev = node;
            node = node->next;
        }
    }
}

/* ----------------------------------------------------------------------- */
/* ft_split — split string by separator                                     */
/* ----------------------------------------------------------------------- */
char **ft_split(char *s, char c)
{
    char **result;
    int count;
    int i;
    int j;
    int pos;

    count = 0;
    i = 0;
    while (s[i])
    {
        while (s[i] == c)
            i++;
        if (s[i])
        {
            count++;
            while (s[i] && s[i] != c)
                i++;
        }
    }
    result = (char **)malloc(sizeof(char *) * (count + 1));
    if (!result)
        return (NULL);
    i = 0;
    j = 0;
    while (s[i])
    {
        while (s[i] == c)
            i++;
        if (s[i])
        {
            pos = i;
            while (s[i] && s[i] != c)
                i++;
            result[j] = (char *)malloc(sizeof(char) * (i - pos + 1));
            if (!result[j])
                return (NULL);
            int k = 0;
            while (pos < i)
            {
                result[j][k++] = s[pos++];
            }
            result[j][k] = 0;
            j++;
        }
    }
    result[j] = NULL;
    return (result);
}

/* ----------------------------------------------------------------------- */
/* rev_wstr — reverse words in string                                      */
/* ----------------------------------------------------------------------- */
char *rev_wstr(char *str)
{
    int len;
    int i;
    char *word_start;
    char *word_end;
    char temp;

    len = ft_strlen(str);
    i = 0;
    while (i < len / 2)
    {
        temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
        i++;
    }
    i = 0;
    while (str[i])
    {
        while (str[i] && str[i] != ' ')
            i++;
        word_start = &str[i];
        while (str[i] == ' ')
            i++;
        word_end = &str[i];
        while (word_end > word_start)
        {
            temp = *word_start;
            *word_start = *(word_end - 1);
            *(word_end - 1) = temp;
            word_start++;
            word_end--;
        }
    }
    return (str);
}

/* ----------------------------------------------------------------------- */
/* rostring — rotate string by one position                                */
/* ----------------------------------------------------------------------- */
void rostring(char *str)
{
    int len;
    int i;
    char first;

    len = ft_strlen(str);
    if (len <= 1)
        return ;
    first = str[0];
    i = 0;
    while (str[i + 1])
    {
        str[i] = str[i + 1];
        i++;
    }
    str[i] = first;
    write(1, str, len);
    write(1, "\n", 1);
}

/* ----------------------------------------------------------------------- */
/* sort_int_tab — sort array in ascending order                            */
/* ----------------------------------------------------------------------- */
void sort_int_tab(int *tab, int size)
{
    int i;
    int j;
    int temp;

    i = 0;
    while (i < size - 1)
    {
        j = i + 1;
        while (j < size)
        {
            if (tab[i] > tab[j])
            {
                temp = tab[i];
                tab[i] = tab[j];
                tab[j] = temp;
            }
            j++;
        }
        i++;
    }
}

/* ----------------------------------------------------------------------- */
/* sort_list — sort linked list by content                               */
/* ----------------------------------------------------------------------- */
void sort_list(list_t *list)
{
    list_t *node;
    list_t *next;
    int swapped;
    int c1;
    int c2;
    void *temp;

    if (!list || !list->next)
        return ;
    do
    {
        swapped = 0;
        node = list;
        while (node && node->next)
        {
            memcpy(&c1, node->content, sizeof(int));
            memcpy(&c2, node->next->content, sizeof(int));
            next = node->next;
            if (c1 > c2)
            {
                temp = node->content;
                node->content = node->next->content;
                node->next->content = temp;
                swapped = 1;
            }
            node = next;
        }
    }
    while (swapped);
}
