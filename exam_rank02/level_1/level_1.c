/* ************************************************************************** */
/*                                                                          */
/*                                                       :::      ::::::::   */
/*   level_1.c                                          :+:      :+:    :+:   */
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
/* first_word — print the first word of a string                           */
/* ----------------------------------------------------------------------- */
int first_word(char *str)
{
    int i;

    i = 0;
    while (str[i] && str[i] != ' ')
    {
        write(1, &str[i], 1);
        i++;
    }
    return (i);
}

/* ----------------------------------------------------------------------- */
/* fizzbuzz — print FizzBuzz from 1 to 100                                 */
/* ----------------------------------------------------------------------- */
void fizzbuzz(void)
{
    int i;
    char *fizz = "Fizz";
    char *buzz = "Buzz";
    char *fb = "FizzBuzz";

    i = 1;
    while (i <= 100)
    {
        if (i % 15 == 0)
            write(1, fb, 8);
        else if (i % 3 == 0)
            write(1, fizz, 4);
        else if (i % 5 == 0)
            write(1, buzz, 4);
        else
        {
            char c = '0' + (i / 10);
            char d = '0' + (i % 10);
            if (i < 10)
            {
                write(1, &d, 1);
            }
            else
            {
                write(1, &c, 1);
                write(1, &d, 1);
            }
        }
        write(1, "\n", 1);
        i++;
    }
}

/* ----------------------------------------------------------------------- */
/* ft_putstr — print a string to stdout                                    */
/* ----------------------------------------------------------------------- */
void ft_putstr(char *str)
{
    int i;

    i = 0;
    while (str[i])
    {
        write(1, &str[i], 1);
        i++;
    }
}

/* ----------------------------------------------------------------------- */
/* ft_strcpy — copy a string to dest                                       */
/* ----------------------------------------------------------------------- */
char *ft_strcpy(char *dest, char *src)
{
    int i;

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
/* ft_strlen — get the length of a string                                  */
/* ----------------------------------------------------------------------- */
int ft_strlen(char *str)
{
    int i;

    i = 0;
    while (str[i])
        i++;
    return (i);
}

/* ----------------------------------------------------------------------- */
/* ft_swap — swap two integers                                             */
/* ----------------------------------------------------------------------- */
void ft_swap(int *a, int *b)
{
    int temp;

    temp = *a;
    *a = *b;
    *b = temp;
}

/* ----------------------------------------------------------------------- */
/* repeat_alpha — print each char repeated                                  */
/* ----------------------------------------------------------------------- */
void repeat_alpha(char *str)
{
    int i;
    int count;

    i = 0;
    while (str[i])
    {
        count = 0;
        while (str[i] >= 'a' && str[i] <= 'z')
        {
            write(1, &str[i], 1);
            count++;
            i++;
        }
        while (str[i] >= 'A' && str[i] <= 'Z')
        {
            write(1, &str[i], 1);
            count++;
            i++;
        }
        if (count == 0)
            i++;
    }
}

/* ----------------------------------------------------------------------- */
/* rev_print — print a string in reverse                                    */
/* ----------------------------------------------------------------------- */
void rev_print(char *str)
{
    int len;

    len = ft_strlen(str) - 1;
    while (len >= 0)
    {
        write(1, &str[len], 1);
        len--;
    }
}

/* ----------------------------------------------------------------------- */
/* rot_13 — rotate characters by 13                                        */
/* ----------------------------------------------------------------------- */
void rot_13(char *str)
{
    int i;

    i = 0;
    while (str[i])
    {
        if (str[i] >= 'a' && str[i] <= 'z')
            str[i] = 'a' + ((str[i] - 'a' + 13) % 26);
        else if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] = 'A' + ((str[i] - 'A' + 13) % 26);
        i++;
    }
    write(1, str, ft_strlen(str));
    write(1, "\n", 1);
}

/* ----------------------------------------------------------------------- */
/* rotone — rotate characters by 1                                         */
/* ----------------------------------------------------------------------- */
void rotone(char *str)
{
    int i;

    i = 0;
    while (str[i])
    {
        if (str[i] >= 'a' && str[i] <= 'z')
            str[i] = 'a' + ((str[i] - 'a' + 1) % 26);
        else if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] = 'A' + ((str[i] - 'A' + 1) % 26);
        i++;
    }
    write(1, str, ft_strlen(str));
    write(1, "\n", 1);
}

/* ----------------------------------------------------------------------- */
/* search_and_replace — replace char a with char b in str                  */
/* ----------------------------------------------------------------------- */
void search_and_replace(char a, char b, char *str)
{
    int i;

    i = 0;
    while (str[i])
    {
        if (str[i] == a)
            str[i] = b;
        i++;
    }
}

/* ----------------------------------------------------------------------- */
/* ulstr — convert to lowercase and sort characters                        */
/* ----------------------------------------------------------------------- */
char *ulstr(char *str)
{
    int len;
    int i;
    int j;
    char c;

    len = ft_strlen(str);
    i = 0;
    while (i < len)
    {
        if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] = str[i] + 32;
        i++;
    }
    i = 0;
    while (i < len - 1)
    {
        j = i + 1;
        while (j < len)
        {
            if (str[i] > str[j])
            {
                c = str[i];
                str[i] = str[j];
                str[j] = c;
            }
            j++;
        }
        i++;
    }
    return (str);
}
