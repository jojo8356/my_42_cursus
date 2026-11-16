/* ************************************************************************** */
/*                                                                          */
/*                                                            :::      ::::::::   */
/*   minishell.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             */
/*   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr       */
/*                                                                          */
/* ************************************************************************** */

# ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <signal.h>
# include <wait.h>
# include <dirent.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <errno.h>
# include "readline_mock.h"
# include "../libft/libft.h"

/* ----------------------- Global variable ------------------------------ */
extern volatile sig_atomic_t g_signal;

/* ----------------------- Token types ----------------------------------- */
enum e_token_type
{
    TOKEN_WORD = 0,
    TOKEN_PIPE,
    TOKEN_INPUT,
    TOKEN_OUTPUT,
    TOKEN_APPEND,
    TOKEN_HEREDOC,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_END
};

/* ----------------------- Token structure ------------------------------ */
typedef struct s_token
{
    char *value;
    int type;
    struct s_token *next;
} token_t;

/* ----------------------- Redirection types ----------------------------- */
enum e_redir_type
{
    REDIR_INPUT = 0,
    REDIR_OUTPUT,
    REDIR_APPEND,
    REDIR_HEREDOC
};

/* ----------------------- Redirection structure ------------------------- */
typedef struct s_redir
{
    int type;
    char *filename;
    int fd;
    char *heredoc_content;
    struct s_redir *next;
} redir_t;

/* ----------------------- Command structure ----------------------------- */
typedef struct s_cmd
{
    char **argv;
    redir_t *redirs;
    struct s_cmd *next;
} cmd_t;

/* ----------------------- Logical types ---------------------------------- */
enum e_logical
{
    LOGICAL_NONE = 0,
    LOGICAL_AND,
    LOGICAL_OR
};

/* ----------------------- Pipeline structure ---------------------------- */
typedef struct s_pipe
{
    cmd_t *first;
    int logical;
    struct s_pipe *next;
} pipe_t;

/* ----------------------- Function declarations ------------------------- */
/* 0_main.c */
int main(int ac, char **av);
int run_shell(void);
char *get_prompt(void);

/* 1_tokenizer.c */
token_t *tokenize_input(char *input);
token_t *create_token(char *value, int type);
int is_operator_char(char c);
int is_quote(char c);

/* 2_parser.c */
pipe_t *parse_tokens(token_t *tokens);
pipe_t *create_pipeline(void);
cmd_t *create_cmd(void);
redir_t *create_redir(int type, char *filename);

/* 3_expander.c */
void expand_pipeline(pipe_t *pipeline, int exit_code);
char **expand_wildcards(char **argv);
char *expand_word(char *word, int exit_code);
char *remove_quotes(char *word);

/* 4_executor.c */
int execute_pipeline(pipe_t *pipeline);
int execute_external(cmd_t *cmd);
int run_single_cmd(cmd_t *cmd, int fd_in, int fd_out);
int run_pipeline_multi(cmd_t *first, int fd_in, int fd_out);
int count_cmds(cmd_t *cmd);
int apply_redirections(cmd_t *cmd);

/* 5_builtins.c */
int is_builtin(char *name);
int exec_builtin(cmd_t *cmd, int *exit_code);
int exec_echo(char **argv);
int exec_cd(char **argv);
int exec_pwd(char **argv);
int exec_export(char **argv);
int exec_unset(char **argv);
int exec_env(char **argv);
int exec_exit(char **argv);

/* 6_redirections.c */
int apply_input_redir(redir_t *redir);
int apply_output_redir(redir_t *redir);
int apply_heredoc_redir(redir_t *redir);

/* 7_signals.c */
void setup_signals(void);
void signal_handler(int sig);

/* 8_cleanup.c */
void free_tokens(token_t *tokens);
void free_pipeline(pipe_t *pipeline);
void free_cmd(cmd_t *cmd);
void free_redirs(redir_t *redirs);
void free_argv(char **argv);

/* bonus */
int execute_with_logical(pipe_t *pipeline);
char **expand_wildcards(char **argv);

/* shell_state.c */
void request_exit(int code);
int should_exit(void);
int get_exit_code(void);
void set_last_status(int status);
int get_last_status(void);

/* read_word / read_operator */
char *read_word(char *input, int *i);
token_t *read_operator(char *input, int *i);
void append_token(token_t **tokens, token_t *token);
void argv_append(char ***argv, char *arg);
int argv_count(char **argv);
void redirs_append(redir_t **redirs, redir_t *redir);
int is_redir_token(int type);

/* signal reset */
void reset_signals(void);

# endif
