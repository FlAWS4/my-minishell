/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:31 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 17:04:25 by my42             ###   ########.fr       */
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
# include <limits.h>

# define BUFFER_SIZE 10

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
# define BOLD_RESET "\033[1;0m"
# define BOLD_MAGENTA "\033[1;35m"

/* Error types */
#define ERROR_SYNTAX    1
#define ERROR_COMMAND   2
#define ERROR_PERMISSION 3
#define ERROR_MEMORY    4
# define RESET "\033[0m"

/* Global variable for signal handling (as allowed by subject) */
extern int	g_signal;

/* Error code enum for better error handling */
typedef enum e_error_code
{
    ERR_NONE,
    ERR_SYNTAX,
    ERR_NOT_FOUND,
    ERR_PERMISSION,
    ERR_PIPE,
    ERR_FORK,
    ERR_MEMORY,
    ERR_REDIR
}	t_error_code;

/* Token types for lexer/parser */
typedef enum e_token_type
{
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIR_IN,
    TOKEN_REDIR_OUT,
    TOKEN_REDIR_APPEND,
    TOKEN_HEREDOC,
    TOKEN_WHITESPACE,
    TOKEN_SINGLE_QUOTE,
    TOKEN_DOUBLE_QUOTE,
}	t_token_type;

typedef struct s_redirection
{
    int                     type;  // REDIR_IN, REDIR_OUT, HEREDOC
    char                    *word;
    struct s_redirection    *next;
} t_redirection;

/* Token structure */
typedef struct s_token
{
    t_token_type	type;
    char			*value;
    struct s_token	*next;
    int             preceded_by_space; // For handling whitespace
}	t_token;

typedef struct s_cmd
{
    char            **args;
    char            *input_file;    // Keep for backward compatibility
    char            *output_file;   // Keep for backward compatibility
    int             append_mode;    // Keep for backward compatibility
    char            *heredoc_delim; // Keep for single heredoc support
    t_redirection   *redirections;  // For multiple redirections
    
    // Add these important fields
    int             input_fd;       // File descriptor for input
    int             output_fd;      // File descriptor for output
    pid_t           pid;            // Process ID for waiting
    
    struct s_cmd    *next;
}   t_cmd;

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
    int		should_exit;
}	t_shell;

/* Environment functions */
t_env	*create_env_node(char *key, char *value);
void	add_env_var(t_env **env_list, t_env *new_node);
void	split_env_string(char *str, char **key, char **value);
t_env	*init_env(char **envp);
char	*get_env_value(t_env *env, const char *key);
t_env	*find_env_var(t_env *env, const char *key);
int     set_env_var(t_env **env_list, char *key, char *value);
int     delete_env_var(t_env **env_list, char *key);
int	    count_env_vars(t_env *env);
void	free_env_array(char **array, int count);
int     is_valid_var_name(char *name);

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
int		ft_isalnum(int c);
int		ft_isdigit(int c);
void	ft_putendl_fd(char *s, int fd);
size_t	ft_strlcat(char *dst, const char *src, size_t size);
void	ft_putchar_fd(char c, int fd);
int		is_whitespace(char c);
char	*get_next_line(int fd);
void	ft_putnbr_fd(int n, int fd);
void    reset_gnl_buffer(void);
int     ft_count_char(const char *str, char c);
int     ft_str_is_numeric(const char *str);
int     ft_isalpha(int c);
char	*ft_itoa(int n);
int	    ft_strncmp(const char *s1, const char *s2, size_t n);

/* Signal handling */
void	setup_signals(void);
void	setup_signals_noninteractive(void);
void	setup_signals_heredoc(void);

/* Lexer functions */
int		handle_word(char *input, int i, t_token **tokens);
t_token	*process_tokens(char *input);
t_token	*tokenize(char *input);
int		handle_quotes(char *input, int i, char quote, t_token **tokens);
int     is_special(char c);
int	    handle_special(char *input, int i, t_token **tokens);

/* Token management functions */
t_token	*create_token(t_token_type type, char *value, int preceded_by_space);
void	add_token(t_token **list, t_token *new);
int	    handle_quote(char *input, int i, t_token **tokens);
t_token	*get_last_token(t_token *tokens);

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
int		process_redirections(t_cmd *cmd);
char	*read_heredoc_line(void);
int		collect_heredoc_input(char *delimiter, int fd);
void    add_redirection(t_cmd *cmd, int type, char *word);
int     process_input_redir(t_redirection *redir);
int     process_output_redir(t_redirection *redir);
int     process_heredoc_redir(t_redirection *redir);
int     process_single_redir(t_redirection *redir);
int	    is_redirection_token(t_token *token);
void    free_redirection_list(t_redirection *redirections);
int	    collect_and_discard_heredoc(char *delimiter);
int	    check_heredoc_line(char *line, char *delimiter, int fd);
int     create_heredoc_file(void);
int     handle_input_redirection(char *filename);
int     handle_output_redirection(char *filename, int append_mode);

/* Token parsing */
void    handle_word_token(t_cmd *cmd, t_token **token);
t_cmd	*handle_pipe_token(t_cmd *current);
int		process_token(t_token **token, t_cmd **current);
t_cmd	*parse_tokens(t_token *tokens, t_shell *shell);
int		validate_syntax(t_token *tokens);
void    merge_adjacent_quoted_tokens(t_token **tokens);

/* Executor functions */
char	*find_command(t_shell *shell, char *cmd);
int		execute_builtin(t_shell *shell, t_cmd *cmd);
void	execute_child(t_shell *shell, t_cmd *cmd);
int		execute_command(t_shell *shell, t_cmd *cmd);
char	*create_path(char *dir, char *cmd);
void	process_cmd_status(t_shell *shell, int status);
char	**env_to_array(t_env *env);
int     is_executable(char *path);
char    *find_command_in_path(char *cmd, t_env *env);
int     wait_for_children(t_shell *shell);

/* Built-in command functions */
int		builtin_echo(t_cmd *cmd);
int		builtin_cd(t_shell *shell, t_cmd *cmd);
int		builtin_export(t_shell *shell, t_cmd *cmd);
int		builtin_unset(t_shell *shell, t_cmd *cmd);
int		builtin_env(t_shell *shell);
int		builtin_exit(t_shell *shell, t_cmd *cmd);
int		is_builtin(char *cmd);
int		execute_pipeline(t_shell *shell, t_cmd *cmd);
void	print_sorted_env(t_shell *shell);
int		builtin_clear(void);
int		builtin_pwd(t_shell *shell, t_cmd *cmd);
int     builtin_help(t_shell *shell);

/* Main.c functions */
void	setup_terminal(void);
void	process_input(t_shell *shell, char *input);
void	shell_loop(t_shell *shell);
t_shell	*init_shell(char **envp);
t_cmd	*parse_input(char *input);
void	execute_parsed_commands(t_shell *shell);
void    free_shell(t_shell *shell);
void    handle_pending_signals(t_shell *shell);

/* Expander functions */
char    *expand_variables(t_shell *shell, char *str);
char	*expand_one_var(t_shell *shell, char *str, int *i);
void	free_expansion_parts(char *name, char *value, char **parts);
void	expand_variables_in_tokens(t_token *tokens, t_shell *shell);

/* Expander utilities */
char	*get_var_name(char *str);
char	*get_var_value(t_shell *shell, char *name);
void	expand_token_variables(t_shell *shell, t_token *tokens);

/* Free functions */
void	free_token_list(t_token *tokens);
void	free_cmd_list(t_cmd *cmd);
void	free_str_array(char **array);

/* Error functions */
void	print_error(char *cmd, char *msg);
void	display_error(int error_type, char *command, char *message);
int     get_error_exit_status(int error_type);
void    print_syntax_error(char *token_value, int token_type);

/* Extra functions */
void	ft_display_welcome(void);
void	create_prompt(char *prompt, int exit_status);
void	save_history_to_file(const char *filename);
void	load_history_from_file(const char *filename);

/* History functions */
void	init_history(void);
void	save_history(void);
void	add_to_history(char *cmd);

/* Debugging functions */
void	print_tokens(t_token *tokens);

#endif
