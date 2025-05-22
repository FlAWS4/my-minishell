/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:31 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/22 17:42:18 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/wait.h>
# include <signal.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <errno.h>
# include <string.h>
# include <sys/stat.h>
# include <termios.h>

// Text colors
# define BLACK "\033[0;30m"
# define RED "\033[0;31m"
# define GREEN "\033[0;32m"
# define YELLOW "\033[0;33m"
# define BLUE "\033[0;34m"
# define PURPLE "\033[0;35m"
# define CYAN "\033[0;36m"
# define WHITE "\033[0;37m"

// Bold text colors
# define BOLD_BLACK "\033[1;30m"
# define BOLD_RED "\033[1;31m"
# define BOLD_GREEN "\033[1;32m"
# define BOLD_YELLOW "\033[1;33m"
# define BOLD_BLUE "\033[1;34m"
# define BOLD_PURPLE "\033[1;35m"
# define BOLD_CYAN "\033[1;36m"
# define BOLD_WHITE "\033[1;37m"

// Reset color
# define RESET "\033[0m"

/* Global variable for signal handling (as allowed by subject) */
extern int	g_signal;

/* Token types for lexer/parser */
typedef enum e_token_type
{
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIR_IN,
    TOKEN_REDIR_OUT,
    TOKEN_REDIR_APPEND,
    TOKEN_HEREDOC
}	t_token_type;

/* Token structure */
typedef struct s_token
{
    t_token_type	type;
    char			*value;
    struct s_token	*next;
}	t_token;

/* Command structure */
typedef struct s_cmd
{
    char			**args;
    char			*input_file;
    char			*output_file;
    int				append_mode;
    char			*heredoc_delim;
    struct s_cmd	*next;
}	t_cmd;

/* Environment structure */
typedef struct s_env
{
    char			*key;
    char			*value;
    struct s_env	*next;
}	t_env;

/* Main shell structure */
typedef struct s_shell
{
    t_env	*env;
    t_cmd	*cmd;
    int		exit_status;
    int    should_exit;
}	t_shell;

/* Environment functions */
t_env	*create_env_node(char *key, char *value);
void	add_env_var(t_env **env_list, t_env *new_node);
void	split_env_string(char *str, char **key, char **value);
t_env	*init_env(char **envp);
char	*get_env_value(t_env *env, const char *key);

/* String utility functions */
char	*ft_strdup(const char *s);
int		ft_strcmp(const char *s1, const char *s2);
size_t	ft_strlen(const char *s);
char	*ft_substr(char const *s, unsigned int start, size_t len);
size_t	ft_strlcpy(char *dst, const char *src, size_t size);
char	**ft_split(const char *s, char c);
char	*ft_strjoin(const char *s1, const char *s2);
char	*ft_strjoin_free(char *s1, const char *s2);
void	ft_putstr_fd(char *s, int fd);
void	ft_bzero(void *s, size_t n);
void	*ft_calloc(size_t nmemb, size_t size);
int	    ft_isalnum(int c);
int	    ft_isdigit(int c);
void	ft_putendl_fd(char *s, int fd);
size_t	ft_strlcpy(char *dst, const char *src, size_t size);
size_t	ft_strlcat(char *dst, const char *src, size_t size);
void	ft_putchar_fd(char c, int fd);


/* Signal handling */
void	setup_signals(void);
void	setup_signals_noninteractive(void);
void	setup_signals_heredoc(void);

/* Lexer functions */
int		handle_word(char *input, int i, t_token **tokens);
t_token	*process_tokens(char *input);
t_token	*tokenize(char *input);

/* Command creation and management */
t_cmd	*create_cmd(void);
int		init_args(t_cmd *cmd, char *arg);
void	add_arg(t_cmd *cmd, char *arg);

/* Redirection handling */
int		handle_redir_in(t_token **token, t_cmd *cmd);
int		handle_redir_out(t_token **token, t_cmd *cmd, int append);
int		handle_heredoc(t_token **token, t_cmd *cmd);
int		parse_redirections(t_token **tokens, t_cmd *cmd);
int		setup_redirections(t_cmd *cmd);

/* Token parsing */
void	handle_word_token(t_cmd *cmd, t_token *token);
t_cmd	*handle_pipe_token(t_cmd *current);
int		process_token(t_token **token, t_cmd **current);
t_cmd	*parse_tokens(t_token *tokens);

/* Executor functions */
char	*find_command(t_shell *shell, char *cmd);
int		execute_builtin(t_shell *shell, t_cmd *cmd);
void	execute_child(t_shell *shell, t_cmd *cmd);
int		execute_command(t_shell *shell, t_cmd *cmd);
char    *create_path(char *dir, char *cmd);

/* Built-in command functions */
int		builtin_echo(t_cmd *cmd);
int		builtin_cd(t_shell *shell, t_cmd *cmd);
int		builtin_export(t_shell *shell, t_cmd *cmd);
int		builtin_unset(t_shell *shell, t_cmd *cmd);
int		builtin_env(t_shell *shell);
int		builtin_exit(t_shell *shell, t_cmd *cmd);
int     is_builtin(char *cmd);
int	    execute_pipeline(t_shell *shell, t_cmd *cmd);
void	print_sorted_env(t_shell *shell);
int	    builtin_clear(void);
int     builtin_pwd(t_shell *shell, t_cmd *cmd);

/*main.c functions*/
void	setup_terminal(void);
void	process_input(t_shell *shell, char *input);
void	shell_loop(t_shell *shell);

/*expander functions*/
char	*expand_variables(t_shell *shell, char *str);
void	expand_token_variables(t_shell *shell, t_token *tokens);

/* Free functions */
void	free_token_list(t_token *tokens);
void	free_cmd_list(t_cmd *cmd);
void	free_shell(t_shell *shell);

/*error functions*/
void	print_error(char *cmd, char *msg);
void    free_str_array(char **array);

/*extra functions*/
void    display_welcome_message(void);
void    create_prompt(char *prompt, int exit_status);
int     builtin_help(t_shell *shell);

#endif