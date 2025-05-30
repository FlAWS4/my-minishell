/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 19:56:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 01:00:49 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process command status from waitpid
 * Sets shell exit status based on child process termination
 */
void	process_cmd_status(t_shell *shell, int status)
{
    int	sig;

    shell->exit_status = 1;
    if (WIFEXITED(status))
    {
        shell->exit_status = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        sig = WTERMSIG(status);
        if (sig == SIGINT)
        {
            ft_putstr_fd("\n", 1);
            shell->exit_status = 130;
        }
        else if (sig == SIGQUIT)
        {
            ft_putstr_fd("Quit (core dumped)\n", 1);
            shell->exit_status = 131;
        }
        else
            shell->exit_status = 128 + sig;
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

    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ECHOCTL;
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
    cmd = parse_tokens(tokens);
    free_token_list(tokens);
    return (cmd);
}

/**
 * Process user input and execute commands
 */
void	process_input(t_shell *shell, char *input)
{
    t_token	*tokens;

    if (!input || input[0] == '\0')
        return;
    tokens = tokenize(input);
    if (!tokens)
        return;
    expand_variables_in_tokens(tokens, shell);
    shell->cmd = parse_tokens(tokens);
    free_token_list(tokens);
    if (shell->cmd)
        execute_parsed_commands(shell);
    free_cmd_list(shell->cmd);
    shell->cmd = NULL;
}
