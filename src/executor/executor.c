/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/29 00:07:47 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


/**
 * Set up redirections for builtin command
 */
static int	setup_builtin_redirections(t_cmd *cmd, int *saved_stdin, 
                                    int *saved_stdout)
{
    *saved_stdin = dup(STDIN_FILENO);
    *saved_stdout = dup(STDOUT_FILENO);
    if (*saved_stdin == -1 || *saved_stdout == -1)
        return (1);
    if (setup_redirections(cmd) != 0)
    {
        close(*saved_stdin);
        close(*saved_stdout);
        return (1);
    }
    return (0);
}

/**
 * Restore original stdin/stdout after builtin
 */
static void	restore_redirections(int saved_stdin, int saved_stdout)
{
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdin);
    close(saved_stdout);
}

/**
 * Execute builtin command with proper redirections
 */
static int	run_builtin_command(t_shell *shell, t_cmd *cmd, char *cmd_name)
{
    if (ft_strcmp(cmd_name, "echo") == 0)
        return (builtin_echo(cmd));
    else if (ft_strcmp(cmd_name, "cd") == 0)
        return (builtin_cd(shell, cmd));
    else if (ft_strcmp(cmd_name, "pwd") == 0)
        return (builtin_pwd(shell, cmd));
    else if (ft_strcmp(cmd_name, "export") == 0)
        return (builtin_export(shell, cmd));
    else if (ft_strcmp(cmd_name, "unset") == 0)
        return (builtin_unset(shell, cmd));
    else if (ft_strcmp(cmd_name, "env") == 0)
        return (builtin_env(shell));
    else if (ft_strcmp(cmd_name, "exit") == 0)
        return (builtin_exit(shell, cmd));
    else if (ft_strcmp(cmd_name, "help") == 0)
        return (builtin_help(shell));
    return (1);
}

/**
 * Execute a shell builtin command
 */
int	execute_builtin(t_shell *shell, t_cmd *cmd)
{
    char	*cmd_name;
    int		saved_stdin;
    int		saved_stdout;
    int		result;

    if (!cmd->args || !cmd->args[0])
        return (1);
    if (setup_builtin_redirections(cmd, &saved_stdin, &saved_stdout) != 0)
        return (1);
    cmd_name = cmd->args[0];
    result = run_builtin_command(shell, cmd, cmd_name);
    restore_redirections(saved_stdin, saved_stdout);
    return (result);
}
/**
 * Handle error when command is not found
static void	handle_command_not_found(char *cmd, char **env_array)
{
    display_error(ERROR_COMMAND, cmd, "command not found");
    free_str_array(env_array);
    exit(127);
}

  Handle execution errors after execve fails

static void	handle_execution_error(char *cmd, char *cmd_path, char **env_array)
{
    free(cmd_path);
    free_str_array(env_array);
    if (errno == ENOEXEC)
        display_error(ERROR_COMMAND, cmd, "not an executable");
    else if (errno == EACCES)
        display_error(ERROR_PERMISSION, cmd, "permission denied");
    else
        display_error(0, cmd, strerror(errno));
    exit(126);
}
*/ 

/**
 * Execute command in child process
 */
void	execute_child(t_shell *shell, t_cmd *cmd)
{
    char	*command_path;
    char	**env_array;

    setup_signals_noninteractive();
    ft_putstr_fd("DEBUG: Executing command: ", 2);
    ft_putstr_fd(cmd->args[0], 2);
    ft_putstr_fd("\n", 2);
    
    if (setup_redirections(cmd) != 0)
        exit(1);
    
    // NO DEBUG READ HERE! It consumes stdin before cat can read it
    
    command_path = find_command(shell, cmd->args[0]);
    if (!command_path)
    {
        display_error(ERR_NOT_FOUND, cmd->args[0], NULL);
        exit(127);
    }
    env_array = env_to_array(shell->env);
    execve(command_path, cmd->args, env_array);
    display_error(0, cmd->args[0], strerror(errno));
    free_env_array(env_array, count_env_vars(shell->env));
    free(command_path);
    exit(126);
}

/**
 * Check if command is a builtin
 */
int	is_builtin(char *cmd)
{
    if (!cmd)
        return (0);
    return (
        ft_strcmp(cmd, "echo") == 0 ||
        ft_strcmp(cmd, "cd") == 0 ||
        ft_strcmp(cmd, "pwd") == 0 ||
        ft_strcmp(cmd, "export") == 0 ||
        ft_strcmp(cmd, "unset") == 0 ||
        ft_strcmp(cmd, "env") == 0 ||
        ft_strcmp(cmd, "exit") == 0 ||
        ft_strcmp(cmd, "help") == 0
    );
}
