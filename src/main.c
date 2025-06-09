/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/09 23:04:15 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	g_signal;

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
        tmp = redirections;
        redirections = redirections->next;
        free(tmp);
    }
}


/**
 * Free all shell resources
 */
void free_shell(t_shell *shell)
{
    t_env *env;
    t_env *next_env;
    
    if (!shell)
        return;
    
    // Free command list
    if (shell->cmd)
        free_cmd_list(shell->cmd);
    
    // Free environment list
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
    
    // Free shell structure
    free(shell);
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
 * Handle pending signals
 */
void	handle_pending_signals(t_shell *shell)
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
 * Parse input into commands - UPDATED to pass shell
 */
t_cmd *parse_input(char *input, t_shell *shell)
{
    t_token *tokens;
    t_cmd *cmd;

    if (!input || !*input)
        return (NULL);
        
    // Pass shell to tokenize_and_expand for variable expansion
    tokens = tokenize_and_expand(input, shell); 
    if (!tokens)
        return (NULL);
    
    // Check for syntax errors in tokens
    if (!validate_syntax(tokens))
    {
        // Set exit status to 2 for syntax errors
        shell->exit_status = 2;
        free_token_list(tokens);
        return (NULL);
    }
        
    // Pass shell to parse_tokens for proper command handling
    cmd = parse_tokens(tokens, shell);
    free_token_list(tokens);
    return (cmd);
}


void execute_parsed_commands(t_shell *shell)
{
    if (!shell || !shell->cmd) {
        printf("DEBUG: No commands to execute\n");
        return;
    }
    
    // Debug the command being executed
    if (shell->cmd->args && shell->cmd->args[0])
        printf("DEBUG: About to execute command: %s\n", shell->cmd->args[0]);
    else
        printf("DEBUG: Command has no args\n");
    
    // Execute the command properly handling pipes and heredocs
    int result = execute(shell, shell->cmd);
    printf("DEBUG: Command execution returned: %d\n", result);
    
    // Ensure all output is flushed
    fflush(stdout);
    fflush(stderr);
}

/**
 * Process input and execute commands - UPDATED to use updated parse_input
 */
void process_input(t_shell *shell, char *input)
{
    // Clear any previous command data
    if (shell->cmd)
    {
        free_cmd_list(shell->cmd);
        shell->cmd = NULL;
    }
    
    // Parse input with the shell context
    shell->cmd = parse_input(input, shell);
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
 * Main shell loop
 */
void	shell_loop(t_shell *shell)
{
    char	*input;
    char	prompt[100];

    while (!shell->should_exit)
    {
        setup_signals();
        handle_pending_signals(shell);
        create_prompt(prompt, shell->exit_status);
        input = readline(prompt);
        if (!input)
        {
            ft_putstr_fd("exit\n", 1);
            break;
        }
        if (input[0] != '\0')
            add_history(input);
        process_input(shell, input);
        free(input);
    }
}

/**
 * Main function
 */
int	main(int argc, char **argv, char **envp)
{
    t_shell	*shell;

    (void)argc;
    (void)argv;
    g_signal = 0;  // Initialize global signal variable
    
    shell = init_shell(envp);
    if (!shell)
        return (1);
    setup_signals();
    setup_terminal();
    ft_display_welcome();
    shell_loop(shell);
    free_shell(shell);
    return (shell->exit_status);
}
