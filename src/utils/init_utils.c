/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 19:56:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 03:47:28 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process command exit status
 */
void process_cmd_status(t_shell *shell, int status)
{
    // Add safety check
    if (!shell)
        return;
        
    if (WIFEXITED(status))
    {
        shell->exit_status = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        // Handle the case where the command was terminated by a signal
        if (WTERMSIG(status) == SIGINT)
            ft_putstr_fd("\n", STDOUT_FILENO);
        shell->exit_status = 128 + WTERMSIG(status);
    }
    
    // Force reset terminal state to ensure prompt displays correctly
    if (isatty(STDIN_FILENO))
    {
        struct termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag |= (ECHO | ECHOE | ICANON | ISIG);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }
}

/**
 * Initialize the shell
 */
t_shell	*init_shell(char **envp)
{
    t_shell	*shell;

    shell = malloc(sizeof(t_shell));
    if (!shell)
        return (NULL);
        
    shell->env = init_env(envp);
    if (!shell->env && envp && *envp)
    {
        free(shell);
        return (NULL);
    }
    
    // Initialize terminal settings
    if (isatty(STDIN_FILENO))
        tcgetattr(STDIN_FILENO, &shell->orig_termios);
    
    shell->cmd = NULL;
    shell->exit_status = 0;
    shell->should_exit = 0;
    g_signal = 0;
    return (shell);
}

/**
 * Set up the terminal attributes
 */
void	setup_terminal(void)
{
    struct termios	term;

    if (tcgetattr(STDIN_FILENO, &term) == -1)
        return;
    term.c_lflag &= ~ECHOCTL;  // Disable control char echoing
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/**
 * Parse input into commands
 */
t_cmd	*parse_input(char *input)
{
    t_token	*tokens;
    t_cmd	*cmd;

    if (!input || !*input)
        return (NULL);
    tokens = tokenize(input);
    if (!tokens)
        return (NULL);
    cmd = parse_tokens(tokens, NULL); // This function should handle NULL shell
    free_token_list(tokens);
    return (cmd);
}
/**
 * Process input and execute commands
 */
void process_input(t_shell *shell, char *input)
{
    // Clear any previous command data
    if (shell->cmd)
    {
        free_cmd_list(shell->cmd);
        shell->cmd = NULL;
    }
    
    // Parse input
    shell->cmd = parse_input(input);
    if (!shell->cmd)
        return;
    
    // Execute commands
    execute_parsed_commands(shell);
    
    // Reset terminal state using the saved original settings
    if (isatty(STDIN_FILENO))
        tcsetattr(STDIN_FILENO, TCSANOW, &shell->orig_termios);
    
    // Clear the command list after execution
    if (shell->cmd)
    {
        free_cmd_list(shell->cmd);
        shell->cmd = NULL;
    }
}

/**
 * Execute parsed commands
 */
void execute_parsed_commands(t_shell *shell)
{
    if (!shell || !shell->cmd)
        return;
        
    // Only try to execute if we have arguments
    if (shell->cmd->args && shell->cmd->args[0])
    {
        // Execute the command and get the result
        execute_command(shell, shell->cmd);
        
        // Ensure all output is flushed
        write(STDOUT_FILENO, "", 0);
        fflush(stdout);
        fflush(stderr);
    }
    
    // Handle signals properly
    if (g_signal)
    {
        shell->exit_status = 130;
        g_signal = 0;
    }
    
    // Reset terminal for next prompt
    setup_signals();
}
