/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/20 21:00:51 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	g_signal;

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
    g_signal = 0;
    return (shell);
}

/**
 * Free command list
 */
void	free_cmd_list(t_cmd *cmd)
{
    t_cmd	*tmp;
    int		i;

    while (cmd)
    {
        if (cmd->args)
        {
            i = 0;
            while (cmd->args[i])
                free(cmd->args[i++]);
            free(cmd->args);
        }
        if (cmd->input_file)
            free(cmd->input_file);
        if (cmd->output_file)
            free(cmd->output_file);
        if (cmd->heredoc_delim)
            free(cmd->heredoc_delim);
        tmp = cmd;
        cmd = cmd->next;
        free(tmp);
    }
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
 * Process user input and execute commands
 */
void	process_input(t_shell *shell, char *input)
{
    t_token	*tokens;

    if (!*input)
        return;
    add_history(input);
    tokens = tokenize(input);
    shell->cmd = parse_tokens(tokens);
    free_token_list(tokens);
    if (shell->cmd && shell->cmd->args)
    {
        execute_command(shell, shell->cmd);
        if (g_signal == 1)
        {
            shell->exit_status = 130;
            g_signal = 0;
        }
    }
    free_cmd_list(shell->cmd);
    shell->cmd = NULL;
    setup_signals();
}

/**
 * Main shell loop
 */
void	shell_loop(t_shell *shell)
{
    char	*input;

    while (1)
    {
        input = readline("minishell$ ");
        if (!input)
        {
            write(1, "exit\n", 5);
            break;
        }
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
    shell = init_shell(envp);
    setup_signals();
    setup_terminal();
    shell_loop(shell);
    free_shell(shell);
    return (shell->exit_status);
}
