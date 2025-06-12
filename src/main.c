/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/12 23:05:07 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int g_signal;

/**
 * Free redirection list
 */
void free_redirection_list(t_redirection *redirections)
{
    t_redirection *tmp;

    while (redirections)
    {
        if (redirections->word)
            free(redirections->word);
        if (redirections->temp_file)
            free(redirections->temp_file);
        tmp = redirections;
        redirections = redirections->next;
        free(tmp);
    }
}

/**
 * Free environment variables
 */
static void free_env_vars(t_shell *shell)
{
    t_env *env;
    t_env *next_env;

    env = shell->env;
    while (env)
    {
        next_env = env->next;
        if (env->key)
            free(env->key);
        if (env->value)
            free(env->value);
        free(env);
        env = next_env;
    }
}

/**
 * Free all shell resources
 */
void free_shell(t_shell *shell)
{
    if (!shell)
        return;
    if (shell->cmd)
        free_cmd_list(shell->cmd);
    free_env_vars(shell);
    free(shell);
}

/**
 * Initialize the shell
 */
t_shell *init_shell(char **envp)
{
    t_shell *shell;

    shell = malloc(sizeof(t_shell));
    if (!shell)
        return (NULL);
    
    shell->env = init_env(envp);
    if (!shell->env && envp && *envp)
    {
        free(shell);
        return (NULL);
    }
    
    if (isatty(STDIN_FILENO))
        tcgetattr(STDIN_FILENO, &shell->orig_termios);
    
    shell->cmd = NULL;
    shell->exit_status = 0;
    shell->should_exit = 0;
    g_signal = 0;
    return (shell);
}

/**
 * Set up terminal for shell operation
 */
void setup_terminal(t_shell *shell)
{
    struct termios term;

    if (!isatty(STDIN_FILENO) || tcgetattr(STDIN_FILENO, &term) == -1)
        return;
    
    if (shell && !shell->orig_termios.c_lflag)
        shell->orig_termios = term;
    
    term.c_lflag |= ECHOCTL;  // Show ^C for proper visual feedback
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/**
 * Restore terminal to shell mode after command execution
 */
void restore_terminal_settings(t_shell *shell)
{
    if (!isatty(STDIN_FILENO) || !shell)
        return;
    
    signal(SIGTTOU, SIG_IGN);
    tcsetattr(STDIN_FILENO, TCSANOW, &shell->orig_termios);
    signal(SIGTTOU, SIG_DFL);
}

/**
 * Process command exit status
 */
void process_cmd_status(t_shell *shell, int status)
{
    if (!shell)
        return;
    
    if (WIFEXITED(status))
        shell->exit_status = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
    {
        if (WTERMSIG(status) == SIGINT)
            ft_putstr_fd("\n", STDOUT_FILENO);
        shell->exit_status = 128 + WTERMSIG(status);
    }
    
    // Reset terminal state for interactive use
    if (isatty(STDIN_FILENO))
    {
        struct termios term;
        
        signal(SIGTTOU, SIG_IGN);
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag |= (ECHO | ECHOE | ICANON | ISIG);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
        signal(SIGTTOU, SIG_DFL);
    }
}

/**
 * Handle pending signals
 */
void handle_pending_signals(t_shell *shell)
{
    if (g_signal)
    {
        if (g_signal == SIGINT)
            shell->exit_status = 130;
        else if (g_signal == SIGQUIT)
            shell->exit_status = 131;
        g_signal = 0;
    }
}

/**
 * Parse input into commands
 */
t_cmd *parse_input(char *input, t_shell *shell)
{
    t_token *tokens;
    t_cmd *cmd;

    if (!input || !*input)
        return (NULL);
    
    tokens = tokenize_and_expand(input, shell);
    if (!tokens)
        return (NULL);
    
    if (!validate_syntax(tokens))
    {
        shell->exit_status = 2;
        free_token_list(tokens);
        return (NULL);
    }
    
    cmd = parse_tokens(tokens, shell);
    free_token_list(tokens);
    return (cmd);
}

/**
 * Execute parsed commands
 */
void execute_parsed_commands(t_shell *shell)
{
    if (!shell || !shell->cmd)
        return;
    execute(shell, shell->cmd);
}

/**
 * Process input and execute commands
 */
void process_input(t_shell *shell, char *input)
{
    // Clean up any previous command
    if (shell->cmd)
    {
        free_cmd_list(shell->cmd);
        shell->cmd = NULL;
    }
    
    // Parse input
    shell->cmd = parse_input(input, shell);
    if (!shell->cmd)
        return;
    
    // Execute command if not interrupted
    if (g_signal != SIGINT)
        execute_parsed_commands(shell);
    else
        g_signal = 0;
    
    // Clean up command
    if (shell->cmd)
    {
        free_cmd_list(shell->cmd);
        shell->cmd = NULL;
    }
}

/**
 * Main shell loop
 */
void shell_loop(t_shell *shell)
{
    char *input;
    char prompt[100];
    
    // Initial terminal setup
    setup_terminal(shell);
    
    // Ensure shell has terminal control (once)
    if (isatty(STDIN_FILENO))
    {
        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO, getpid());
        signal(SIGTTOU, SIG_DFL);
    }

    while (!shell->should_exit)
    {
        // Setup signals for interactive use
        setup_signals();
        
        // Handle interrupt signal
        if (g_signal == SIGINT)
        {
            g_signal = 0;
            shell->exit_status = 130;
            continue;
        }
        
        // Display prompt and get input
        create_prompt(prompt, shell->exit_status);
        input = readline(prompt);
        
        // Handle EOF (Ctrl+D)
        if (!input)
        {
            ft_putstr_fd("exit\n", 1);
            shell->should_exit = 1;
            break;
        }
        
        // Process non-empty input
        if (input[0] != '\0')
            add_history(input);
        
        process_input(shell, input);
        free(input);
    }
}

/**
 * Program entry point
 */
int main(int argc, char **argv, char **envp)
{
    t_shell *shell;

    (void)argc;
    (void)argv;
    g_signal = 0;
    
    shell = init_shell(envp);
    if (!shell)
        return (1);
    
    setup_signals();
    setup_terminal(shell);  // Pass shell to setup_terminal
    ft_display_welcome();
    shell_loop(shell);
    
    // Restore terminal and free resources
    restore_terminal_settings(shell);
    free_shell(shell);
    
    return (shell->exit_status);
}
