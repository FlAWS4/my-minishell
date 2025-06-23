/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 18:36:54 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	check_null_command(t_shell *shell, t_command *cmd)
{
	if (!cmd)
		handle_cmd_error(shell, NULL, "internal error (null command)",
			EXIT_FAILURE);
	if (!cmd->args || !cmd->args[0] || cmd->args[0][0] == '\0')
		handle_cmd_error(shell, "", "command not found", 127);
}

static char	*check_absolute_path(t_shell *shell, char *cmd_path)
{
	if (access(cmd_path, F_OK) != 0)
		handle_cmd_error(shell, cmd_path, "No such file or directory\n", 127);
	if (access(cmd_path, X_OK) != 0)
		handle_cmd_error(shell, cmd_path, "Permission denied", 126);
	return (cmd_path);
}

char	*find_executable(char *cmd, t_shell *shell)
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

static char	*find_in_path(t_shell *shell, char *cmd_name)
{
	char	*executable_path;

	executable_path = find_executable(cmd_name, shell);
	if (!executable_path)
		handle_cmd_error(shell, cmd_name, "command not found", 127);
	return (executable_path);
}

char	*get_command_path(t_shell *shell, t_command *cmd)
{
	char	*executable_path;

	check_null_command(shell, cmd);
	redirect_stdio(cmd);
	if (!cmd->args || !cmd->args[0] || !writable(STDOUT_FILENO, cmd->args[0]))
		clean_and_exit_shell(shell, 1);
	if (ft_strchr(cmd->args[0], '/'))
		executable_path = check_absolute_path(shell, cmd->args[0]);
	else
		executable_path = find_in_path(shell, cmd->args[0]);
	return (executable_path);
}

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

static void	check_dev_full(t_command *cmd)
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

void	apply_redirs_and_run_builtin(t_shell *shell, t_command *cmd)
{
	if (handle_redirections(cmd, shell) == -1)
		return ;
	redirect_stdio(cmd);
	if (!cmd->args || !cmd->args[0] || !writable(STDOUT_FILENO, cmd->args[0]))
		return (restore_std_fds(shell), g_exit_status = 1, (void)0);
	if (!ft_strcmp(cmd->args[0], "exit"))
		return (builtin_exit(shell, cmd), restore_std_fds(shell), (void)0);
	g_exit_status = run_builtin(shell, cmd);
	check_dev_full(cmd);
	restore_std_fds(shell);
}

static void	update_exit_after_wait(pid_t pid)
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

static void	ignore_sigint_and_wait(pid_t child_pid)
{
	void	(*original_handler)(int);

	original_handler = signal(SIGINT, SIG_IGN);
	update_exit_after_wait(child_pid);
	signal(SIGINT, original_handler);
}

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

static void	execute_in_child(t_shell *shell, t_command *cmd)
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

void	execute_single_command(t_shell *shell, t_command *cmd)
{
	pid_t	child_pid;

	if (is_builtin(cmd))
	{
		apply_redirs_and_run_builtin(shell, cmd);
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
		execute_in_child(shell, cmd);
	else
		ignore_sigint_and_wait(child_pid);
}
