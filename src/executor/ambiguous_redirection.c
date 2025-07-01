/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirection_list.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 15:42:56 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 15:42:56 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * open_file_for_input - Opens a file in read-only mode
 * @path: Path of the file to open
 * 
 * This function attempts to open a file for reading input,
 * which is used for input redirection (< file).
 * If the file cannot be opened, it calls report_file_error to report
 * the specific error and returns -1.
 * 
 * Return: File descriptor on success, -1 on failure
 */
int	open_file_for_input(const char *path)
{
	int	fd;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		return (report_file_error(path));
	return (fd);
}

/**
 * open_file_for_output - Opens a file for writing, truncating if it exists
 * @path: Path of the file to open or create
 * 
 * This function attempts to open a file for writing output,
 * which is used for output redirection (> file).
 * Creates the file if it doesn't exist, truncates if it does.
 * Sets standard permissions (0666 modified by umask).
 * 
 * Return: File descriptor on success, -1 on failure
 */
int	open_file_for_output(const char *path)
{
	int	fd;

	fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
	if (fd == -1)
		return (report_file_error(path));
	return (fd);
}

/**
 * open_file_for_append - Opens a file for appending data
 * @path: Path of the file to open or create
 * 
 * This function attempts to open a file for appending output,
 * which is used for append redirection (>> file).
 * Creates the file if it doesn't exist, preserves content if it does.
 * Sets standard permissions (0644 modified by umask).
 * 
 * Return: File descriptor on success, -1 on failure
 */
int	open_file_for_append(const char *path)
{
	int	fd;

	fd = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd == -1)
		return (report_file_error(path));
	return (fd);
}
/**
 * create_ambiguous_redirect_error - 
 * Creates a redirection node for ambiguous redirect
 * @redirs: Pointer to redirection list where the error node will be added
 * @tokens: Token that triggered the ambiguous redirect error
 * 
 * This function creates a special redirection node that represents an
 * ambiguous redirection error. It reports the error to the user and
 * sets the global exit status. The ar (ambiguous redirect) flag is set
 * to prevent further processing of this redirection.
 * 
 * Return: 0 on success, 1 on memory allocation failure
 */

int	create_ambiguous_redirect_error(t_redir \
	**redirs, t_token *tokens)
{
	t_redir	*redir;

	if (tokens->type == HEREDOC)
		return (0);
	redir = malloc(sizeof(t_redir));
	if (!redir)
		return (1);
	ft_memset(redir, 0, sizeof(t_redir));
	redir->type = tokens->type;
	ft_putstr_fd("minishell: ambiguous redirect\n", 2);
	g_exit_status = 1;
	redir->ar = 1;
	redir->next = NULL;
	add_redirs(redirs, redir);
	return (0);
}

/**
 * detect_ambiguous_redirect - 
 * Checks if a redirection token has an ambiguous target
 * @cmd: Redirection structure to mark as ambiguous if needed
 * @tokens: Token sequence to analyze for ambiguity
 * 
 * This function examines tokens to determine if a redirection is ambiguous.
 * A redirection is considered ambiguous when:
 * 1. A redirection operator isn't followed by a word token
 * 2. A redirection target word was previously marked as ambiguous
 * 
 * When an ambiguous redirection is detected, it reports the error and
 * marks the command's redirection as ambiguous to prevent execution.
 */
void	detect_ambiguous_redirect(t_redir *cmd, t_token *tokens)
{
	t_token	*token;

	token = tokens;
	if (token->type == HEREDOC)
		return ;
	if (token->type == REDIR_IN || token->type == REDIR_OUT
		|| token->type == APPEND)
	{
		if (token->next && token->next->type != WORD)
		{
			ft_putstr_fd("minishell: ambiguous redirect\n", 2);
			g_exit_status = 1;
			cmd->ar = 1;
			return ;
		}
	}
	if (token->next && token->next->type == WORD && token->next->ar)
	{
		ft_putstr_fd("minishell: ambiguous redirect\n", 2);
		g_exit_status = 1;
		cmd->ar = 1;
		return ;
	}
}
