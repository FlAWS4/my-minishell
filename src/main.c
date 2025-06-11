/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/11 03:30:28 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	g_signal;

/**
 * Free redirection list
 */
void	free_redirection_list(t_redirection *redirections)
{
    t_redirection	*tmp;

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
static void	free_env_vars(t_shell *shell)
{
    t_env	*env;
    t_env	*next_env;

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
 * Free all shell resources
 */
void	free_shell(t_shell *shell)
{
    if (!shell)
        return ;
    if (shell->cmd)
        free_cmd_list(shell->cmd);
    free_env_vars(shell);
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
        return ;
    term.c_lflag &= ~ECHOCTL;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/**
 * Reset terminal state after command execution
 */
static void	reset_terminal_state(void)
{
    struct termios	term;

    if (isatty(STDIN_FILENO))
    {
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag |= (ECHO | ECHOE | ICANON | ISIG);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }
}

/**
 * Process command exit status
 */
void	process_cmd_status(t_shell *shell, int status)
{
    if (!shell)
        return ;
    if (WIFEXITED(status))
        shell->exit_status = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
    {
        if (WTERMSIG(status) == SIGINT)
            ft_putstr_fd("\n", STDOUT_FILENO);
        shell->exit_status = 128 + WTERMSIG(status);
    }
    reset_terminal_state();
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
 * Parse input into commands
 */
t_cmd	*parse_input(char *input, t_shell *shell)
{
    t_token	*tokens;
    t_cmd	*cmd;

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
void	execute_parsed_commands(t_shell *shell)
{
    if (!shell || !shell->cmd)
        return ;
    execute(shell, shell->cmd);
}

/**
 * Process input and execute commands
 */
void	process_input(t_shell *shell, char *input)
{
    if (shell->cmd)
    {
        free_cmd_list(shell->cmd);
        shell->cmd = NULL;
    }
    shell->cmd = parse_input(input, shell);
    if (!shell->cmd)
        return ;
    execute_parsed_commands(shell);
    if (isatty(STDIN_FILENO))
        tcsetattr(STDIN_FILENO, TCSANOW, &shell->orig_termios);
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

    while (!shell->should_exit)
    {
        // Reset signals first, then check for SIGINT
        setup_signals();
        
        if (g_signal == SIGINT)
        {
            // Reset signal flag immediately
            g_signal = 0;
            shell->exit_status = 130;
            
            // Use readline functions to ensure clean prompt state
            rl_replace_line("", 0);
            rl_on_new_line();
            
            // Reset terminal settings
            if (isatty(STDIN_FILENO))
                tcsetattr(STDIN_FILENO, TCSANOW, &shell->orig_termios);
                
            continue;
        }
        // Create prompt and get input
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
    g_signal = 0;
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
