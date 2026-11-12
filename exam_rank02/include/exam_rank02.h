#ifndef EXAM_RANK02_H
# define EXAM_RANK02_H

# include <stddef.h>

/* ===== Level 1 ===== */
int first_word(char *str);
void fizzbuzz(void);
void ft_putstr(char *str);
char *ft_strcpy(char *dest, char *src);
int ft_strlen(char *str);
void ft_swap(int *a, int *b);
void repeat_alpha(char *str);
void rev_print(char *str);
void rot_13(char *str);
void rotone(char *str);
void search_and_replace(char a, char b, char *str);
char *ulstr(char *str);

/* ===== Level 2 ===== */
char *alpha_mirror(char *str);
char *camel_to_snake(char *str);
int do_op(int a, int b, char op);
int ft_atoi(char *str);
int ft_strcmp(char *s1, char *s2);
int ft_strcspn(char *s, char *accept);
char *ft_strdup(char *src);
char *ft_strpbrk(char *s1, char *s2);
char *ft_strrev(char *src);
int ft_strspn(char *s, char *accept);
int inter(int ac, char **av);
char *last_word(char *str);
int is_power_of_2(unsigned int n);
int max(int tab[], int size);
void print_bits(unsigned int nb);
void reverse_bits(unsigned int nb);
char *snake_to_camel(char *str);
void swap_bits(void);
char *union_words(char *s1, char *s2);
int wdmatch(char *s1, char *s2);

/* Linked list type for exam exercises */
typedef struct s_list {
    void *content;
    struct s_list *next;
} list_t;

/* ===== Level 3 ===== */
int add_prime_sum(int ac, char **av);
char *epur_str(char *str);
char *expand_str(char *str);
int ft_atoi_base(char *str, char *base);
int ft_list_size(list_t *list);
char **ft_range(int min, int max);
char **ft_rrange(int min, int max);
void hidenp(char *s1, char *s2);
int lcm(int a, int b);
int paramsum(char *str);
int pgcd(int a, int b);
void print_hex(unsigned int nb);
char *rstr_capitalizer(char *str);
char *str_capitalizer(char *str);
char **tab_mult(int nb);

/* ===== Level 4 ===== */
void flood_fill(int y, int x, int value, int **map, int height, int width);
char *fprime(int nb);
char *ft_itoa(int n);
void ft_list_foreach(list_t *list, void (*f)(void *));
void ft_list_remove_if(list_t *list, int cmp(void *, void *), void (*f)(void *));
char **ft_split(char *s, char c);
char *rev_wstr(char *str);
void rostring(char *str);
void sort_int_tab(int *tab, int size);
void sort_list(list_t *list);

#endif
