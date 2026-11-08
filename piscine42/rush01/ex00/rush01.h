/* Rush01 : resolution de grille gratte-ciel (4x4 a 9x9, bonus). */
#ifndef RUSH01_H
# define RUSH01_H

typedef struct s_ctx
{
	int	n;
	int	clues[36];
	int	grid[81];
	int	*cand[9];
	int	cnt[9];
	int	col_used[9];
}	t_ctx;

int		parse_clues(char *s, t_ctx *g);
int		visible_count(int *line, int n, int from_end);
int		col_top_ok(t_ctx *g, int depth, int *row, int c);
int		max_unused(t_ctx *g, int c, int extra);
int		bot_contrib(int *col, int depth, int max_s);
int		col_bot_ok(t_ctx *g, int depth, int *row, int c);
int		build_rows(t_ctx *g);
void	filter_rows(t_ctx *g);
int		solve(t_ctx *g, int depth);
void	free_rows(t_ctx *g);
void	print_grid(t_ctx *g);
void	print_error(void);

#endif
