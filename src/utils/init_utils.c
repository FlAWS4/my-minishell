/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 19:56:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/24 19:59:10 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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

    if (!*input)
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
    if (!*input)
        return ;
    
    shell->cmd = parse_input(input);
    if (!shell->cmd)
    {
        ft_putstr_fd("minishell: syntax error\n", 2);
        shell->exit_status = 2;
        return ;
    }
    
    execute_parsed_commands(shell);
    free_cmd_list(shell->cmd);
    shell->cmd = NULL;
    setup_signals();
}
