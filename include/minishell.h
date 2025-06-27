/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:31 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 20:00:45 by hchowdhu         ###   ########.fr       */
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
# include <sys/ioctl.h>
# include <termios.h>
# include <stdint.h> 

# define PROMPT_SIZE 256
# define OPEN_MAX 1024

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
# define RESET "\033[0m"

# define GC_FATAL 1
# define GC_SOFT 0
# define BUFFER_SIZE 5

# define ERROR_MALLOC    "minishell: error: memory allocation failed\n"
# define ERROR_ARGS      "minishell: error: no arguments allowed\n"
# define ERROR_QUOTES    "minishell: syntax error: unclosed quote\n"
# define ERROR_ENV_INIT  "minishell: error: failed to initialize environment\n"
# define ERROR_TOKENIZE  "minishell: error: failed to create token list\n"
# define ERROR_SYNTAX    "minishell: syntax error near unexpected token\n"
# define ERROR_SYNTAX_NL "minishell: syntax error near \
unexpected token 'newline'\n"
# define ERROR_SYNTAX_PIPE "minishell: syntax error near unexpected token '|'"
# define ERROR_HEREDOC_EOF "minishell: warning: here-document delimited by \
end-of-file (wanted `"
# define ERROR_UNSUPPORTED "minishell: error: unsupported character\n"
# define ERROR_IDENTIFIER  "minishell: error: not a valid identifier\n"

# define GC_FATAL 1
# define GC_SOFT 0
# define BUFFER_SIZE 5

extern int	g_exit_status;

typedef struct s_gc
{
	void			*data;
	int				flag;
	void			(*free_array)(void *);
	struct s_gc		*next;
}	t_gc;

typedef enum e_token_type
{
	WORD,
	PIPE,
	REDIR_IN,
	REDIR_OUT,
	APPEND,
	HEREDOC,
	T_EOF,
	ERROR,
}	t_token_type;

typedef struct s_token
{
	t_token_type	type;
	char			*value;
	int				single_quote;
	int				double_quote;
	int				space_before;
	int				space_after;
	int				ar;
	int				quoted_outside;
	struct s_token	*previous;
	struct s_token	*next;
}	t_token;

typedef struct s_redir
{
	t_token_type	type;
	char			*file_or_del;
	char			*heredoc_content;
	int				quoted;
	int				quoted_outside;
	int				ar;
	struct s_redir	*next;
}	t_redir;

typedef struct s_int
{
	int	start;
	int	end;
	int	i;
}	t_int;

typedef struct s_char
{
	char	*line;
	char	*str;
	char	*new_line;
}	t_char;

typedef struct s_command
{
	char				**args;
	t_redir				*redirs;
	int					fd_in;
	int					fd_out;
	pid_t				pid;
	struct s_command	*previous;
	struct s_command	*next;
}	t_command;

typedef struct s_shell
{
	char		**env;
	t_token		*tokens;
	t_command	*commands;
	t_gc		*gc;
	int			saved_stdin;
	int			saved_stdout;
	int			heredoc_interupt;
	int			pipe_interupt;
	char		*default_path;
}	t_shell;

typedef struct s_pipe_data
{
	pid_t	*pids;
	int		*fork_count;
	pid_t	*last_pid;
	int		*input_fd;
}	t_pipe_data;

void	setup_signals(void);
void	handle_interrupt(int sig);
void	reset_signals_to_default(void);
int		safely_execute_command(t_shell *shell);
void	disable_control_char_echo(void);
void	enable_control_char_echo(void);
void	handle_heredoc_interrupt(int sig);
void	setup_heredoc_signal_handlers(struct sigaction *old_int,\
	struct sigaction *old_quit);
void	restore_signal_handlers(struct sigaction *old_int,\
	struct sigaction *old_quit);
void	restore_signals_clear_buffer(struct sigaction *old_int,\
	struct sigaction *old_quit);
void	display_heredoc_eof_warning(char *delim);
void	restore_standard_fds(t_shell *shell);

// BUILTINS
int		builtin_cd(t_shell *shell, t_command *cmd);
int		builtin_echo(t_command *cmd);
int		builtin_env(t_shell *shell, t_command *cmd);
int		builtin_exit(t_shell *shell, t_command *cmd);
int		builtin_export(t_shell *shell, t_command *cmd);
int		builtin_pwd(t_shell *shell);
int		builtin_unset(t_shell *shell, t_command *cmd);
int		builtin_help(t_shell *shell);//Add help command
int		is_builtin(t_command *cmd);
int		run_builtin(t_shell *shell, t_command *cmd);
void	execute_builtin_with_redirections(t_shell *shell, t_command *cmd);

// COMMAND EXEC
void	execute_non_piped_command(t_shell *shell, t_command *cmd);
void	execute_command_sequence(t_shell *shell);
void	ignore_sigint_and_wait(pid_t child_pid);
void	setup_and_execute_child_process(t_shell *shell, t_command *cmd);
void	execute_pipe(t_shell *shell, t_command *cmd, pid_t *pids);
int		create_pipe_if_needed(t_command *cmd, int pipe_fds[2]);
int		fork_pipe_child(t_shell *shell, t_command *cmd,\
	int input_fd, int pipe_fds[2]);

// ENV
char	**get_env(char **envp, t_shell *shell);
char	*get_env_value(t_shell *shell, const char *var_name);
char	**add_env_var(t_shell *shell, char *new_var);
char	**init_env(char **envp, t_shell *shell);
char	**init_minimal_env(void);
int		env_error(const char *msg, t_gc **gc);
int		duplicate_existing_vars(t_shell *shell, char **new_env, int size);
int		add_to_env(t_shell *shell, char *new_var);
int		find_var_pos(char *var_name, t_shell *shell);
int		is_valid_identifier(const char *str);
int		env_has_path(char **envp);
void	sort_env_for_export(char **env_copy);
void	update_env(t_shell *shell, char *var, char *new_value);

// EXEC PATH
char	*search_path_for_exec(char *cmd, t_shell *shell);
char	*get_command_path(t_shell *shell, t_command *cmd);

// HEREDOC
char	*capture_heredoc(t_redir *redirs, t_shell *data);
int		handle_heredoc(t_command *cmd, t_redir *redir);
char	*get_next_line(int fd, int clear);
char	*read_and_store(char *buffer, int fd);
char	*update_buffer(char *buffer);
char	*extract_line(char *buffer);

// PIPE
void	prepare_pipe_execution(t_shell *shell, t_command *cmd);
char	*join_args(t_shell *shell, char **args);
int		pipe_and_fork_one_cmd(t_shell *shell, t_command *cmd,
			t_pipe_data *data);
void	wait_for_all_children(pid_t *pids, int count, pid_t last_pid);
void	wait_for_some_children(pid_t *pids, int count);
void	setup_child_fds(t_command *cmd, int input_fd, int pipe_fds[2]);
void	handle_pipe_child(t_shell *shell, t_command *cmd, int input_fd,
			int pipe_fds[2]);

// REDIRECTIONS
int		handle_redirections(t_command *cmd, t_shell *shell);
void	redirect_stdio(t_command *cmd);
int		open_error(const char *filename);
int		open_input_file(const char *path);
int		open_output_file(const char *path);
int		open_append_file(const char *path);
int		open_redir_file(t_redir *redir, t_command *cmd);
void	set_redir_fds(t_command *cmd, int type, int fd);
void	setup_child_output_fd(t_command *cmd, int pipe_fds[2]);
void	setup_child_input_fd(t_command *cmd, int input_fd);

// SHELL MANAGEMENT AND UTILS
void	init_shell_fds(t_shell *shell);
int		is_shell_command(char *cmd);
int		writable(int fd, const char *cmd_name);
void	clean_and_exit_shell(t_shell *shell, int exit_code);
void	update_shell_lvl(t_shell *shell);
void	close_fds(t_shell *shell);
void	error(const char *cmd, const char *error_item, const char *msg);
void	error_quoted(const char *cmd, const char *error_item, const char *msg);
void	handle_cmd_error(t_shell *shell, const char *cmd, const char *msg,
			int exit_code);
void	close_all_fds_except_stdio(void);
void	close_all_command_fds(t_command *all_cmds, t_command *current_cmd);

// GARBAGE COLLECTOR / ERROR HANDLING
char	*gc_strdup(t_gc **gc, const char *s1);
char	*gc_strjoin(t_gc **gc, const char *s1, const char *s2);
char	*gc_substr(t_gc **gc, const char *s, unsigned int start,
			size_t len);
char	**gc_split(t_gc **gc, char *str, char c);
char	*gc_itoa(t_gc **gc, int n);
int		gc_add(t_gc **gc_list, void *ptr, int fatal,
			void (*free_array)(void *));
void	gc_free_all(t_gc **gc_list);
void	gc_free_one(t_gc *gc_list);
void	*gc_malloc(t_gc **gc_list, size_t size, int fatal,
			void (*free_array)(void *));
void	free_array(void *ptr);
void	free_redirs(t_redir **redirs);
void	free_command(t_command **cmd);
void	free_tokens_list(t_token **head);

// TOKENIZER
t_token	*tokenize_input(char *input);
t_token	*create_token(t_token_type type, char *value);
int		handle_in_quote(int start_quote, char *input, int *i, t_token **tokens);
int		handle_double_operator(int *i, t_token **tokens,
			t_token_type operator);
int		handle_single_operator(int *i, t_token **tokens,
			t_token_type operator);
int		is_operator(char *str, int i);
int		is_whitespace(char c);
int		is_quote(char c);
void	add_token(t_token **head, t_token *new_token);
int		is_whitespace_bis(char *str);
void	restore_signals_clear_buffer(struct sigaction *old_int,
			struct sigaction *old_quit);

// SYNTAX CHECK
int		syntax_check(t_shell *cmd);
int		add_token_error(t_token **tokens, t_token *token, char *str);
int		is_token_operator(t_token_type token_type);
int		is_operator_follow(t_token **tokens, t_token *current_token);
int		check_unsupported_character(t_token **tokens);
int		check_token_error(t_token **tokens);
int		check_unsupported_character(t_token **tokens);

// PARSING
t_redir	*init_redir(t_shell *data);
int		split_cmd_with_pipe(t_shell *data);
int		count_pipe(t_shell *data);
int		count_words(t_shell *data);
int		is_cmd(t_token *tokens);
int		fill_words(t_token *tokens, char **args);
int		expand_exit_status(char **result, char *var);
int		join_quoted_str(t_token **tokens);
int		join_no_space(t_token **tokens);
int		expand_token(t_shell *data);
int		prepare_token_str(t_shell *data);
void	remove_useless_token(t_token **tokens, t_token *token);
void	check_ambiguous_redirect(t_redir *cmd, t_token *tokens);
void	add_redirs(t_redir **head, t_redir *redir);
int		add_ambiguous_redirect(t_redir **redirs, t_token *tokens);

// GET NEXT COMMAND
int		end_with_pipe(char *input);
int		read_complete_command(t_shell *data, char **input);

// EXPANSION
char	*expand_variables(t_shell *data, char *input, t_redir *redir);
char	*expand_and_join(t_shell *data, char *args, char *dollar);
char	*is_exist(t_shell *data, char *args);
char	*remove_dollar(char *args);
char	*expand_value(char **env, char *dollar);
char	**copy_var_name(char **tab);
char	**copy_var_value(char **tab);
char	*copy_name(char *str);
char	*copy_value(char *str);
char	*remove_and_replace(char *str, char *expanded);
int		multiple_expand_bis(t_shell *data, char *input, char **str);
int		expand_arguments_bis(t_shell *data, char *input, char **str);
int		expand_status(char *args);
int		potential_expand(char *args, int *i);
int		count_dollars(char *args);
int		expand_arguments(t_shell *data, t_token *tokens, char **args);
int		add_text(char **result, char *var);
int		add_variable(char **result, char *value, int start, int end);
int		check_variable(t_shell *data, char **result, char *value, int *i);
int		multiple_expand(t_shell *data, t_token *tokens, char **args);
int		is_env_variable(t_shell *data, char *dollar);
int		expantion(t_shell *data, t_token *tokens);
int		find_equal(char *str);
void	init_for_norm(int *i, int *start);
int		split_tokens(t_token *current, char *str);
int		expand_token(t_shell *data);

// UTILS

char	*format_shell_prompt(t_shell *shell);
char	*get_redir_file(t_token *tokens, t_redir *redir);
int		is_heredoc(t_redir *redir, t_shell *data);
void	remove_useless_dollars(t_token **head);
void	clean_empty_tokens(t_token **head);
int		count_words_split(const char *s, char c);
char	*ft_strstr(const char *big, const char *little);
void	ft_display_welcome(void);
void	*ft_memset(void *s, int c, size_t n);
void	display_error(int error_type, char *command, char *message);
char	*ft_substr(char const *s, unsigned int start, size_t len);
char	*ft_strjoin(const char *s1, const char *s2);
char	*ft_strdup(const char *s);
int		ft_strcmp(const char *s1, const char *s2);
size_t	ft_strlen(const char *s);
void	ft_bzero(void *s, size_t n);
char	*ft_itoa(int n);
int		ft_isalnum(int c);
char	*ft_strchr(const char *s, int c);
int		ft_isdigit(int c);
char	**ft_split(const char *s, char c);
void	ft_putendl_fd(char *s, int fd);
void	ft_putstr_fd(const char *s, int fd);
int		ft_putchar_fd(char c, int fd);
int		ft_isalpha(int c);
int		ft_strncmp(const char *s1, const char *s2, size_t n);
int		ft_atoi(const char *nptr);
void	ft_putnbr_fd(int n, int fd);
void	*ft_memcpy(void *dest, const void *src, size_t n);
int		ft_safe_size_add(size_t a, size_t b, size_t *result);
void	*ft_calloc(size_t nmemb, size_t size);
size_t	ft_strlcpy(char *dst, const char *src, size_t size);
size_t	ft_strlcat(char *dst, const char *src, size_t size);

#endif
