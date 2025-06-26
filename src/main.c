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
 * dispatch_commands - Execute commands stored in the shell structure
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
void	dispatch_commands(t_shell *shell)
{
    t_command	*cmd;

    cmd = shell->commands;
    if (!cmd)
        return;
    
    // Handle redirection-only commands (no args but with redirections)
    if (!cmd->args && cmd->redirs && !cmd->next)
    {
        g_exit_status = (handle_redirections(cmd, shell) == -1) ? 1 : 0;
        free_command(&shell->commands);
        return;
    }
    
    // Dispatch based on whether we have a pipeline or single command
    if (cmd->next)
        prepare_pipe_execution(shell, cmd);
    else
        execute_non_piped_command(shell, cmd);
        
    free_command(&shell->commands);
}

static int	parse_input(t_shell *shell, char *input)
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

static int	get_input(char **input, const char *prompt)
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

static int	shell_loop(t_shell *shell)
{
	const char	*prompt;
	char		*input;
	int			status;

	prompt = format_shell_prompt(shell);
	while (1)
	{
		setup_signals();
		status = get_input(&input, prompt);
		if (status == 1)
			break ;
		if (status == 2)
			continue ;
		if (parse_input(shell, input))
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
    
    // Validate arguments
    if (argc != 1)
    {
        ft_putstr_fd(BOLD_RED "Usage: ./minishell\n" RESET, STDERR_FILENO);
        return (1);
    }
    
    // Initialize shell structure
    ft_memset(&shell, 0, sizeof(t_shell));
    shell.env = init_env(envp, &shell);
    if (!shell.env)
        clean_and_exit_shell(&shell, 1);
    
    // Save original file descriptors
    shell.saved_stdin = dup(STDIN_FILENO);
    shell.saved_stdout = dup(STDOUT_FILENO);
    if (shell.saved_stdin == -1 || shell.saved_stdout == -1)
    {
        error("dup", NULL, strerror(errno));
        return (1);
    }
    
    // Don't allow piping minishell into itself
    if (!isatty(STDIN_FILENO))
    {
        ft_putstr_fd(BOLD_YELLOW "dont pipe minishell into minishell.\n" RESET, STDERR_FILENO);
        return (0);
    }
    
    // Display welcome message and start shell
    ft_display_welcome();
    shell_loop(&shell);
    clean_and_exit_shell(&shell, g_exit_status);
    
    return (g_exit_status);
}
