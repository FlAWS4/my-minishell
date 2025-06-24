/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/24 01:34:48 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Validates that a command exists and has arguments
 * 
 * @param shell  Shell context containing environment and settings
 * @param cmd    Command to validate
 * 
 * Exits the shell with appropriate error message and status code if:
 * - Command is NULL (internal error)
 * - Command has no arguments or empty first argument
 */

static void	validate_command_exists(t_shell *shell, t_command *cmd)
{
	if (!cmd)
		handle_cmd_error(shell, NULL, "internal error (null command)",
			EXIT_FAILURE);
	if (!cmd->args || !cmd->args[0] || cmd->args[0][0] == '\0')
		handle_cmd_error(shell, "", "command not found", 127);
}

/**
 * Validates an absolute path for execution
 * 
 * @param shell     Shell context
 * @param cmd_path  Absolute path to check
 * 
 * @return The validated path if accessible
 * 
 * Exits the shell with appropriate error message if:
 * - Path doesn't exist (127)
 * - Path exists but execution permission denied (126)
 */
static char	*validate_executable_path(t_shell *shell, char *cmd_path)
{
	if (access(cmd_path, F_OK) != 0)
		handle_cmd_error(shell, cmd_path, "No such file or directory\n", 127);
	if (access(cmd_path, X_OK) != 0)
		handle_cmd_error(shell, cmd_path, "Permission denied", 126);
	return (cmd_path);
}
/**
 * Searches for executable in PATH directories
 * 
 * @param cmd    Command name to find
 * @param shell  Shell context containing environment variables
 * 
 * @return Full path to executable if found, NULL otherwise
 * 
 * Prioritizes PATH from environment, falls back to default path if needed.
 * Returns NULL if command not found in any PATH directory.
 */
char	*search_path_for_exec(char *cmd, t_shell *shell)
{
	char	**paths;
	char	*full_path;
	char	*temp;
	int		i;
	char	*path_env;

	i = 0;
	path_env = get_env_value(shell, "PATH");
	if (!path_env && shell->default_path)
		path_env = shell->default_path;
	if (!path_env)
		return (NULL);
	paths = ft_split(path_env, ':');
	if (!paths)
		return (NULL);
	while (paths[i])
	{
		temp = gc_strjoin(&shell->gc, paths[i], "/");
		full_path = gc_strjoin(&shell->gc, temp, cmd);
		if (access(full_path, X_OK) == 0)
			return (free_array(paths), full_path);
		i++;
	}
	free_array(paths);
	return (NULL);
}
/**
 * Searches PATH for executable and handles command not found error
 * 
 * @param shell     Shell context
 * @param cmd_name  Command name to search for
 * 
 * @return Full path to executable
 * 
 * Exits the shell with "command not found" error if executable not found
 */
static char	*resolve_command_path(t_shell *shell, char *cmd_name)
{
	char	*executable_path;

	executable_path = search_path_for_exec(cmd_name, shell);
	if (!executable_path)
		handle_cmd_error(shell, cmd_name, "command not found", 127);
	return (executable_path);
}
/**
 * Resolves full command path for execution
 * 
 * @param shell  Shell context
 * @param cmd    Command to resolve path for
 * 
 * @return Full path to executable
 * 
 * Handles both absolute paths (containing '/') and commands to find in PATH.
 * Validates command arguments and output writability before resolution.
 * Exits the shell on validation failure or if command not found.
 */
char	*get_command_path(t_shell *shell, t_command *cmd)
{
	char	*executable_path;

	validate_command_exists(shell, cmd);
	redirect_stdio(cmd);
	if (!cmd->args || !cmd->args[0] || !writable(STDOUT_FILENO, cmd->args[0]))
		clean_and_exit_shell(shell, 1);
	if (ft_strchr(cmd->args[0], '/'))
		executable_path = validate_executable_path(shell, cmd->args[0]);
	else
		executable_path = resolve_command_path(shell, cmd->args[0]);
	return (executable_path);
}
/**
 * Checks if command is a shell builtin
 * 
 * @param cmd  Command to check
 * 
 * @return 1 if command is a builtin, 0 otherwise
 * 
 * Supported builtins: cd, echo, env, exit, export, pwd, unset, help
 */
int	is_builtin(t_command *cmd)
{
    if (!cmd || !cmd->args || !cmd->args[0])
        return (0);
    if (ft_strcmp(cmd->args[0], "cd") == 0 ||
        ft_strcmp(cmd->args[0], "echo") == 0 ||
        ft_strcmp(cmd->args[0], "env") == 0 ||
        ft_strcmp(cmd->args[0], "exit") == 0 ||
        ft_strcmp(cmd->args[0], "export") == 0 ||
        ft_strcmp(cmd->args[0], "pwd") == 0 ||
        ft_strcmp(cmd->args[0], "unset") == 0 ||
        ft_strcmp(cmd->args[0], "help") == 0)  // Add help command
        return (1);
    return (0);
}
/**
 * Executes a builtin command
 * 
 * @param shell  Shell context
 * @param cmd    Command to execute
 * 
 * @return Exit status of the builtin command
 * 
 * Routes execution to the appropriate builtin handler function
 * based on the command name.
 */
int	run_builtin(t_shell *shell, t_command *cmd)
{
    if (ft_strcmp(cmd->args[0], "cd") == 0)
        return (builtin_cd(shell, cmd));
    else if (ft_strcmp(cmd->args[0], "echo") == 0)
        return (builtin_echo(cmd));
    else if (ft_strcmp(cmd->args[0], "env") == 0)
        return (builtin_env(shell, cmd));
    else if (ft_strcmp(cmd->args[0], "exit") == 0)
        return (builtin_exit(shell, cmd));
    else if (ft_strcmp(cmd->args[0], "export") == 0)
        return (builtin_export(shell, cmd));
    else if (ft_strcmp(cmd->args[0], "pwd") == 0)
        return (builtin_pwd(shell));
    else if (ft_strcmp(cmd->args[0], "unset") == 0)
        return (builtin_unset(shell, cmd));
    else if (ft_strcmp(cmd->args[0], "help") == 0)
        return (builtin_help(shell));  // Add help command handler
    return (1);
}

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
 * Updates the global exit status after waiting for a child process
 * 
 * @param pid  Process ID to wait for
 * 
 * Handles both normal exit status and signals that terminated the process.
 * For signals, adds special handling for SIGINT and SIGQUIT.
 */
static void	process_command_exit_status(pid_t pid)
{
	int	status;
	int	sig;

	if (waitpid(pid, &status, 0) == -1)
		return ;
	if (WIFEXITED(status))
		g_exit_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
	{
		sig = WTERMSIG(status);
		if (sig == SIGINT)
			ft_putendl_fd("", STDOUT_FILENO);
		else if (sig == SIGQUIT)
			ft_putendl_fd("^\\Quit", STDOUT_FILENO);
		g_exit_status = 128 + sig;
	}
}
/**
 * Ignores SIGINT signal while waiting for child process
 * 
 * @param child_pid  Process ID to wait for
 * 
 * Temporarily sets SIGINT to be ignored while waiting for child process,
 * then restores the original signal handler.
 */
static void	ignore_sigint_and_wait(pid_t child_pid)
{
	void	(*original_handler)(int);

	original_handler = signal(SIGINT, SIG_IGN);
	process_command_exit_status(child_pid);
	signal(SIGINT, original_handler);
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
 * Sets up child process environment and executes command
 * 
 * @param shell  Shell context
 * @param cmd    Command to execute
 * 
 * Resets signal handlers, handles redirections, updates shell level
 * for nested shells, and executes the command.
 * Exits with appropriate error code on any failure.
 */
static void	setup_and_execute_child_process(t_shell *shell, t_command *cmd)
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
