/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:11:33 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	g_exit_status = 0;

/**
 * execute_command_sequence - Execute commands stored in the shell structure
 * @shell: Shell structure containing commands to execute
 *
 * Routes command execution based on command type:
 * 1. Empty command list: Early return
 * 2. Redirection-only command: Handle redirections and update exit status
 * 3. Pipeline (multiple commands): Call pipe execution handler
 * 4. Single command: Execute directly
 * 
 * All commands are freed after execution regardless of success/failure.
 */
void	execute_command_sequence(t_shell *shell)
{
    t_command	*cmd;
    int			redir_status;

    cmd = shell->commands;
    if (!cmd)
        return ;
    if (!cmd->args && cmd->redirs && !cmd->next)
    {
        redir_status = handle_redirections(cmd, shell);
        if (redir_status == -1)
            g_exit_status = 1;
        else
            g_exit_status = 0;
        free_command(&shell->commands);
        return ;
    }
    if (cmd->next)
        prepare_pipe_execution(shell, cmd);
    else
        execute_non_piped_command(shell, cmd);
    free_command(&shell->commands);
}

/**
 * tokenize_and_validate_input - Process user input into executable tokens
 * @shell: Shell structure to store processed tokens
 * @input: Raw command string from user
 *
 * Handles multiline commands with trailing pipes, tokenizes input,
 * validates syntax, and prepares command structure for execution.
 *
 * Return: 0 on success, 1 on error
 */
static int	tokenize_and_validate_input(t_shell *shell, char *input)
{
    if (end_with_pipe(input))
    {
        if (read_complete_command(shell, &input))
            return (1);
    }
    add_history(input);
    shell->tokens = tokenize_input(input);
    if (syntax_check(shell))
        return (free_command(&shell->commands), 1);
    return (0);
}


/**
 * read_user_command - Read command input from the user
 * @input: Pointer to store the read input string
 * @prompt: Shell prompt to display
 *
 * Handles special cases: EOF (ctrl+D), empty commands
 *
 * Return: 0 for successful read, 1 for exit signal, 2 for empty command
 */
static int	read_user_command(char **input, const char *prompt)
{
    *input = readline(prompt);
    if (*input == NULL)
    {
        ft_putstr_fd("exit\n", STDOUT_FILENO);
        return (1);
    }
    if (**input == '\0')
    {
        free(*input);
        return (2);
    }
    return (0);
}

/**
 * run_command_loop - Main shell interaction loop
 * @shell: Shell structure with environment and state
 *
 * Continuously reads, parses and executes commands until exit
 * Handles signal setup before each command and restores file
 * descriptors after command execution.
 *
 * Return: 0 on normal exit
 */
static int	run_command_loop(t_shell *shell)
{
    char	    *prompt;
    char		*input;
    int			status;

    while (1)
    {
        prompt = format_shell_prompt(shell);
        setup_signals();
        status = read_user_command(&input, prompt);
        free (prompt);
        if (status == 1)
            break ;
        if (status == 2)
            continue ;
        if (tokenize_and_validate_input(shell, input))
            continue ;
        if (safely_execute_command(shell))
            continue ;
        restore_standard_fds(shell);
    }
    return (0);
}

/**
 * main - Entry point for minishell program
 * @argc: Argument count
 * @argv: Argument values
 * @envp: Environment variables
 *
 * Initializes the shell environment, sets up signal handlers,
 * duplicates standard file descriptors, checks for invalid usage,
 * and starts the main command loop. Handles cleanup on exit.
 *
 * Return: Exit status of last command or error code
 */
int	main(int argc, char **argv, char **envp)
{
    t_shell	shell;

    (void)argv;
    if (argc != 1)
    {
        ft_putstr_fd(BOLD_RED "Usage: ./minishell\n" RESET, STDERR_FILENO);
        return (1);
    }
    ft_memset(&shell, 0, sizeof(t_shell));
    shell.env = init_env(envp, &shell);
    if (!shell.env)
        clean_and_exit_shell(&shell, 1);
    init_shell_fds(&shell);
    if (!isatty(STDIN_FILENO))
    {
        ft_putstr_fd(BOLD_YELLOW "dont pipe minishell into minishell.\n" RESET,
            STDERR_FILENO);
        return (0);
    }
    ft_display_welcome();
    run_command_loop(&shell);
    clean_and_exit_shell(&shell, g_exit_status);
    return (g_exit_status);
}