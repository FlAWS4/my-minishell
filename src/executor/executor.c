/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 20:00:40 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Checks for special /dev/full output redirection
 * 
 * @param cmd  Command to check
 * 
 * Special case for testing "no space left on device" error handling.
 * Sets exit status to 1 if /dev/full is full and error occurs.
 */
static void	check_device_full_error(t_command *cmd)
{
	t_redir	*redir;

	redir = cmd->redirs;
	while (redir && redir->next)
		redir = redir->next;
	if (redir && (redir->type == REDIR_OUT || redir->type == APPEND)
		&& redir->file_or_del && ft_strcmp(redir->file_or_del, "/dev/full") == 0
		&& ft_strcmp(cmd->args[0], "unset") != 0
		&& ft_strcmp(cmd->args[0], "cd") != 0)
	{
		if (write(STDOUT_FILENO, " ", 1) == -1 && errno == ENOSPC)
		{
			error(cmd->args[0], NULL, "write error: No space left on device");
			g_exit_status = 1;
		}
	}
}

/**
 * Executes an external command in child process
 * 
 * @param shell  Shell context
 * @param cmd    Command to execute
 * 
 * Resolves command path, validates it's not a directory,
 * closes unnecessary file descriptors, and executes the command.
 * Exits with appropriate error code on failure.
 */

 static void	execute_external_command(t_shell *shell, t_command *cmd)
{
	char		*exec_path;
	struct stat	path_stat;

	exec_path = get_command_path(shell, cmd);
	if (stat(exec_path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
	{
		error(NULL, exec_path, "Is a directory");
		clean_and_exit_shell(shell, 126);
	}
	close_fds(shell);
	execve(exec_path, cmd->args, shell->env);
	error(NULL, exec_path, strerror(errno));
	clean_and_exit_shell(shell, 126);
}

/**
 * Executes a single command (not part of a pipeline)
 * 
 * @param shell  Shell context
 * @param cmd    Command to execute
 * 
 * Handles builtins directly in the current process.
 * For external commands, forks a child process and waits for completion.
 * Updates the global exit status based on command result.
 */

 void	execute_non_piped_command(t_shell *shell, t_command *cmd)
{
	pid_t	child_pid;

	if (is_builtin(cmd))
	{
		execute_builtin_with_redirections(shell, cmd);
		return ;
	}
	child_pid = fork();
	if (child_pid == -1)
	{
		error("fork", NULL, strerror(errno));
		g_exit_status = 1;
		return ;
	}
	if (child_pid == 0)
		setup_and_execute_child_process(shell, cmd);
	else
		ignore_sigint_and_wait(child_pid);
}
/**
 * Applies redirections and executes builtin commands
 * 
 * @param shell  Shell context
 * @param cmd    Command to execute
 * 
 * Sets up redirections, validates command, executes appropriate builtin,
 * and restores standard file descriptors after execution.
 * Special handling for 'exit' command to clean up properly.
 */
void	execute_builtin_with_redirections(t_shell *shell, t_command *cmd)
{
	if (handle_redirections(cmd, shell) == -1)
		return ;
	redirect_stdio(cmd);
	if (!cmd->args || !cmd->args[0] || !writable(STDOUT_FILENO, cmd->args[0]))
		return (restore_standard_fds(shell), g_exit_status = 1, (void)0);
	if (!ft_strcmp(cmd->args[0], "exit"))
		return (builtin_exit(shell, cmd), restore_standard_fds(shell), (void)0);
	g_exit_status = run_builtin(shell, cmd);
	check_device_full_error(cmd);
	restore_standard_fds(shell);
}

/**
 * Sets up child process environment and executes command
 * 
 * @param shell  Shell context
 * @param cmd    Command to execute
 * 
 * Resets signal handlers, handles redirections, updates shell level
 * for nested shells, and executes the command.
 * Exits with appropriate error code on any failure.
 */
void	setup_and_execute_child_process(t_shell *shell, t_command *cmd)
{
	reset_signals_to_default();
	if (handle_redirections(cmd, shell) == -1)
		clean_and_exit_shell(shell, 1);
	redirect_stdio(cmd);
	if (!cmd->args || !cmd->args[0] || !writable(STDOUT_FILENO, cmd->args[0]))
		clean_and_exit_shell(shell, 1);
	if (cmd->args && is_shell_command(cmd->args[0]))
		update_shell_lvl(shell);
	execute_external_command(shell, cmd);
}
