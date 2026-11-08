/* Rush02 : conversion d'un nombre en lettres via dictionnaire.
   Bonus : style ecrit (- , and), autre langue (dico FR), lecture stdin. */
#ifndef RUSH02_H
# define RUSH02_H

# define MAX_ENTRIES 512

typedef struct s_dict
{
	unsigned long long	key[MAX_ENTRIES];
	char				*val[MAX_ENTRIES];
	int					count;
	int					lang;
}	t_dict;

int					ft_strlen(char *s);
void				ft_putstr(char *s);
int					ft_strcmp(char *a, char *b);
char				*ft_strdup_n(char *s, int len);
unsigned long long	ft_atoull(char *s, int *ok);

int					dict_load(char *path, t_dict *d);
char				*dict_find(t_dict *d, unsigned long long k);

int					convert(t_dict *d, unsigned long long n, int w, char *out);

#endif
