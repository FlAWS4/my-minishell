/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:31 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/13 21:27:17 by mshariar         ###   ########.fr       */
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
}	t_shell;

/* Function prototypes to be added as you implement them */

/* Environment functions */
t_env	*create_env_node(char *key, char *value);
void	add_env_var(t_env **env_list, t_env *new_node);
void	split_env_string(char *str, char **key, char **value);
t_env	*init_env(char **envp);
char	*get_env_value(t_env *env, char *key);

/* String utility functions */
char	*ft_strdup(const char *s);
int     ft_strcmp(const char *s1, const char *s2);
size_t	ft_strlen(const char *s);

/* Signal handling functions - already defined but adding for completeness */
void	signl_handler(int signum);
void	setup_signals(void);

#endif