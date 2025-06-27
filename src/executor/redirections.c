/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/24 01:43:24 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*join_args(t_shell *shell, char **args)
{
	char	*result;
	char	*tmp;
	int		i;

	i = 0;
	if (!args || !args[0])
		return (gc_strdup(&shell->gc, ""));
	result = gc_strdup(&shell->gc, args[i]);
	i++;
	while (args[i])
	{
		tmp = gc_strjoin(&shell->gc, result, "");
		result = gc_strjoin(&shell->gc, tmp, args[i]);
		i++;
	}
	return (result);
}
int	open_error(const char *filename)
{
	if (errno == EISDIR)
		error(NULL, filename, "Is a directory");
	else if (errno == EACCES)
		error(NULL, filename, "Permission denied");
	else
		error(NULL, filename, strerror(errno));
	return (-1);
}
int	open_redir_file(t_redir *redir, t_command *cmd)
{
	if (redir->ar)
		return (-1);
	 if (redir->type == HEREDOC)
    {
        if (handle_heredoc(cmd, redir) == -1)
            return (-1);
        return (0);
    }
	if (!redir->file_or_del || !*redir->file_or_del)
	{
		error(NULL, "", "No such file or directory\n");
		return (-1);
	}
	if (redir->type == REDIR_IN)
		return (open_input_file(redir->file_or_del));
	else if (redir->type == REDIR_OUT)
		return (open_output_file(redir->file_or_del));
	else if (redir->type == APPEND)
		return (open_append_file(redir->file_or_del));
	else if (redir->type == HEREDOC)
	{
		if (handle_heredoc(cmd, redir) == -1)
			return (-1);
		return (0);
	}
	return (-1);
}


int	handle_redirections(t_command *cmd, t_shell *shell)
{
	t_redir	*redir;
	int		fd;

	redir = cmd->redirs;
	while (redir)
	{
		fd = open_redir_file(redir, cmd);
		if (fd == -1)
		{
			close_fds(shell);
			g_exit_status = 1;
			return (-1);
		}
		if (redir->type != HEREDOC || fd > 0)
			set_redir_fds(cmd, redir->type, fd);
		redir = redir->next;
	}
	return (0);
}

int	handle_builtin_redirections(t_shell *shell, t_command *cmd)
{
	int	saved_stdin;
	int	saved_stdout;
	int	result;

	saved_stdin = -1;
	saved_stdout = -1;
	if (cmd->fd_in != STDIN_FILENO && cmd->fd_in != -1)
		saved_stdin = dup(STDIN_FILENO);
	if (cmd->fd_out != STDOUT_FILENO && cmd->fd_out != -1)
		saved_stdout = dup(STDOUT_FILENO);
	redirect_stdio(cmd);
	result = run_builtin(shell, cmd);
	if (saved_stdin != -1)
	{
		dup2(saved_stdin, STDIN_FILENO);
		close(saved_stdin);
	}
	if (saved_stdout != -1)
	{
		dup2(saved_stdout, STDOUT_FILENO);
		close(saved_stdout);
	}
	return (result);
}
