/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/18 00:05:39 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	g_signal;

/**
 * Parse input string into command structure
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
    cmd = parse_tokens(tokens, shell);
    free_token_list(tokens);
    return (cmd);
}

/**
 * Process command exit status and update shell status
 */
void	process_cmd_status(t_shell *shell, int status)
{
    if (WIFEXITED(status))
        shell->exit_status = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
    {
        shell->exit_status = 128 + WTERMSIG(status);
        if (WTERMSIG(status) == SIGQUIT)
            ft_putendl_fd("Quit (core dumped)", STDERR_FILENO);
    }
}

/**
 * Setup terminal settings for shell
 */
void	setup_terminal(t_shell *shell)
{
    struct termios	term;

    if (!isatty(STDIN_FILENO))
        return ;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ECHOCTL;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    (void)shell;
}

/**
 * Free a list of redirections
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
}

/**
 * Free shell structure and all resources
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
 * Initialize the shell structure
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
    return (shell);
}

/**
 * Process user input into commands and execute
 */
void	process_input(t_shell *shell, char *input)
{
    t_cmd	*cmd;

    cmd = parse_input(input, shell);
    if (!cmd)
        return ;
    if (g_signal != SIGINT)
        execute(shell, cmd);
    else
        g_signal = 0;
    free_cmd_list(cmd);
}

/**
 * Handle input preparation and prompt creation
 */
static char	*get_shell_input(t_shell *shell)
{
    char	prompt[PROMPT_SIZE];
    char	*input;

    if (g_signal == SIGINT)
    {
        g_signal = 0;
        shell->exit_status = 130;
    }
    create_prompt(prompt, shell->exit_status, shell);
    input = readline(prompt);
    if (!input)
    {
        ft_putstr_fd("exit\n", 1);
        shell->should_exit = 1;
        return (NULL);
    }
    return (input);
}

/**
 * Main shell loop
 */
void	shell_loop(t_shell *shell)
{
    char	*input;

    setup_signals();
    setup_terminal(shell);
    while (!shell->should_exit)
    {
        input = get_shell_input(shell);
        if (!input)
            break ;
        if (*input)
            add_history(input);
        process_input(shell, input);
        free(input);
    }
}

/**
 * Main entry point
 */
int	main(int argc, char **argv, char **envp)
{
    t_shell	*shell;
    int		exit_status;

    (void)argc;
    (void)argv;
    g_signal = 0;
    if (!isatty(STDIN_FILENO))
    {
        ft_putstr_fd(BOLD_RED "🚫 Error" RESET ": ", STDERR_FILENO);
        ft_putstr_fd("minishell cannot be run in non-interactive mode\n", 
            STDERR_FILENO);
        ft_putstr_fd(BOLD_YELLOW "💡 Hint" RESET 
            ": go fuck yourself\n", STDERR_FILENO);
        return (1);
    }
    shell = init_shell(envp);
    if (!shell)
        return (1);
    ft_display_welcome();
    shell_loop(shell);
    cleanup_readline_resources();
    restore_terminal_settings(shell);
    exit_status = shell->exit_status;
    free_shell(shell);
    return (exit_status);
}
