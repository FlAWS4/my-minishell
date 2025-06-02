/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:22:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 02:31:24 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if command is an absolute or relative path
 */
static char	*check_direct_path(char *cmd)
{
    if (cmd[0] == '/' || cmd[0] == '.')
        return (ft_strdup(cmd));
    return (NULL);
}

/**
 * Search for command in PATH directories
 */
static char	*search_in_path(char **paths, char *cmd)
{
    char	*full_path;
    int		i;
    
    i = 0;
    while (paths && paths[i])
    {
        full_path = create_path(paths[i], cmd);
        if (full_path && access(full_path, X_OK) == 0)
            return (full_path);
        free(full_path);
        i++;
    }
    return (NULL);
}

/**
 * Find command in PATH
 */
char	*find_command(t_shell *shell, char *cmd)
{
    char	*path_env;
    char	**paths;
    char	*result;
    
    if (!cmd || !*cmd)
        return (NULL);
    
    result = check_direct_path(cmd);
    if (result)
        return (result);
    
    path_env = get_env_value(shell->env, "PATH");
    if (!path_env)
        return (NULL);
        
    paths = ft_split(path_env, ':');
    free(path_env);
    
    result = search_in_path(paths, cmd);
    free_str_array(paths); // Always free paths, not inside search_in_path
    return (result);
}

/**
 * Handle redirections without command
 */
static int	handle_empty_with_redir(t_shell *shell, t_cmd *cmd)
{
    pid_t	pid;
    int		status;

    pid = fork();
    if (pid == 0)
    {
        if (setup_redirections(cmd) != 0)
            exit(1);
        exit(0);
    }
    else if (pid < 0)
    {
        display_error(0, "fork", strerror(errno));
        return (1);
    }
    waitpid(pid, &status, 0);
    process_cmd_status(shell, status);
    return (shell->exit_status);
}

/**
 * Check if command is empty or has empty name
 */
static int	is_empty_command(t_cmd *cmd)
{
    return (!cmd->args || !cmd->args[0] || !*cmd->args[0]);
}

int execute_command(t_shell *shell, t_cmd *cmd)
{
    pid_t pid;
    int status;

    if (!cmd)
        return (1);
    if (is_empty_command(cmd) && (cmd->output_file || cmd->redirections))
        return (handle_empty_with_redir(shell, cmd));
    if (!cmd->args)
        return (1);
    if (cmd->next)
        return (execute_pipeline(shell, cmd));
    if (cmd->args[0] && is_builtin(cmd->args[0]))
        return (execute_builtin(shell, cmd));
    status = 0;
    pid = fork();
    if (pid == 0)
        execute_child(shell, cmd);
    else if (pid < 0)
    {
        display_error(0, "fork", strerror(errno));
        return (1);
    }
    
    // Wait for command to complete
    waitpid(pid, &status, 0);
    
    // Enhanced output synchronization
    write(STDOUT_FILENO, "", 0);
    fflush(stdout);
    fflush(stderr);
    usleep(25000);  // Add a 25ms delay to ensure output is processed
    
    // Cleanup any file descriptors that might be left open
    if (cmd->input_fd > 2)
        close(cmd->input_fd);
    if (cmd->output_fd > 2)
        close(cmd->output_fd);
    
    process_cmd_status(shell, status);
    return (shell->exit_status);
}
