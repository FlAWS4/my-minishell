/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 05:40:45 by my42             ###   ########.fr       */
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


/**
 * Execute child process with proper error handling
 */
void execute_child(t_shell *shell, t_cmd *cmd)
{
    char *command_path;
    char **env_array = NULL;
    
    // Validate command before proceeding
    if (!cmd || !cmd->args || !cmd->args[0]) 
    {
        exit(127);
    }
    
    // Reset signals to default behavior for child processes
    setup_signals_noninteractive();
    
    // Reset terminal attributes for interactive commands
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) == 0)
    {
        term.c_lflag |= (ECHO | ECHOE | ICANON | ISIG);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }
    
    // IMPORTANT: First check if command exists
    command_path = find_command(shell, cmd->args[0]);
    if (!command_path)
    {
        display_error(ERR_NOT_FOUND, cmd->args[0], NULL);
        exit(127);
    }
    
    // Now it's safe to set up redirections
    if (setup_redirections(cmd) != 0)
    {
        free(command_path);
        exit(1);
    }
    
    // Create environment array
    env_array = env_to_array(shell->env);
    if (!env_array)
    {
        free(command_path);
        display_error(ERROR_MEMORY, "env_to_array", "Failed to allocate memory");
        exit(126);
    }
    
    // Execute command
    execve(command_path, cmd->args, env_array);
    
    // Only reached if execve fails
    display_error(ERR_EXEC, command_path, strerror(errno));
    free_env_array(env_array, count_env_vars(shell->env));
    free(command_path);
    exit(126);
}

/**
 * Execute a command with proper signal and terminal handling
 */
/**
 * Execute a command with proper signal and terminal handling
 */
int execute_command(t_shell *shell, t_cmd *cmd)
{
    int status = 0;
    pid_t pid;
    
    if (!cmd || !cmd->args || !cmd->args[0])
        return (1);
    
    // Process heredoc if needed
    if (cmd->heredoc_delim)
    {
        if (!process_heredoc(cmd))
            return (1);
    }
    
    // For builtins, execute directly
    if (is_builtin(cmd->args[0]))
    {
        int ret = execute_builtin(shell, cmd);
        
        // Clean up after builtins
        if (cmd->input_fd != -1)
        {
            close(cmd->input_fd);
            cmd->input_fd = -1;
        }
        
        // CRITICAL FIX: Don't attempt to unlink/free heredoc_file
        // It was already taken care of in process_heredoc
        
        return (ret);
    }
    
    // For external commands
    pid = fork();
    if (pid == 0)
    {
        // Child process
        setup_signals_noninteractive();
        
        // Find command path
        char *cmd_path = find_command(shell, cmd->args[0]);
        if (!cmd_path)
        {
            display_error(ERR_NOT_FOUND, cmd->args[0], NULL);
            exit(127);
        }
        
        // Set up redirections
        if (setup_redirections(cmd) != 0)
        {
            free(cmd_path);
            exit(1);
        }
        
        // Create environment array and execute
        char **env_array = env_to_array(shell->env);
        if (!env_array)
        {
            free(cmd_path);
            exit(126);
        }
        
        execve(cmd_path, cmd->args, env_array);
        
        // Only reached on error
        display_error(ERR_EXEC, cmd_path, strerror(errno));
        free(cmd_path);
        free_env_array(env_array, count_env_vars(shell->env));
        exit(126);
    }
    else if (pid < 0)
    {
        // Fork failed
        display_error(0, "fork", strerror(errno));
        return (1);
    }
    
    // Parent process
    signal(SIGINT, SIG_IGN);
    waitpid(pid, &status, 0);
    setup_signals();
    
    // Clean up resources
    if (cmd->input_fd != -1)
    {
        close(cmd->input_fd);
        cmd->input_fd = -1;
    }
    
    // CRITICAL FIX: Don't attempt to cleanup heredoc_file again
    // It was already handled in process_heredoc
    
    // Process status
    if (WIFEXITED(status))
        shell->exit_status = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
        shell->exit_status = 128 + WTERMSIG(status);
    
    return (shell->exit_status);
}
