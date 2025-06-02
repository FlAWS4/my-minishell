/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 02:36:30 by my42             ###   ########.fr       */
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
 * Write debug message to a file
 */
// Add __attribute__((unused)) to tell the compiler this is intentional
static void debug_to_file(const char *msg, const char *value) __attribute__((unused));
static void debug_to_file(const char *msg, const char *value)
{
    FILE *debug_file = fopen("/tmp/minishell_debug.log", "a");
    if (debug_file)
    {
        fprintf(debug_file, "[DEBUG] %s", msg);
        if (value)
            fprintf(debug_file, ": %s", value);
        fprintf(debug_file, "\n");
        fclose(debug_file);
    }
}

void execute_child(t_shell *shell, t_cmd *cmd)
{
    char *command_path;
    char **env_array;
    struct termios term;
    
    setup_signals_noninteractive();
    
    if (setup_redirections(cmd) != 0)
        exit(1);
    
    // Reset terminal to canonical mode for better output handling
    if (isatty(STDOUT_FILENO))
    {
        tcgetattr(STDOUT_FILENO, &term);
        term.c_lflag |= (ICANON | ECHO);
        tcsetattr(STDOUT_FILENO, TCSANOW, &term);
    }
    
    // Special handling for heredoc or cat commands
    if (cmd->args[0] && (ft_strcmp(cmd->args[0], "cat") == 0 || 
                         cmd->heredoc_delim != NULL)) 
    {
        if (ft_strcmp(cmd->args[0], "cat") == 0 && !cmd->args[1]) 
        {
            // Handle simple cat with heredoc input
            char buffer[4096];
            ssize_t bytes;
            
            while ((bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0) 
            {
                buffer[bytes] = '\0';
                write(STDOUT_FILENO, buffer, bytes);
                // Ensure output is displayed by forcing a flush
                write(STDOUT_FILENO, "", 0);
            }
            
            exit(0);
        }
    }
    
    // Normal command execution
    command_path = find_command(shell, cmd->args[0]);
    if (!command_path)
    {
        display_error(ERR_NOT_FOUND, cmd->args[0], NULL);
        exit(127);
    }
    
    env_array = env_to_array(shell->env);
    execve(command_path, cmd->args, env_array);
    
    // Only reaches here if execve fails
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
