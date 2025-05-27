/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/26 23:45:31 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	g_signal;

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
 * Execute parsed commands
 */
void	execute_parsed_commands(t_shell *shell)
{
    if (shell->cmd && shell->cmd->args)
    {
        execute_command(shell, shell->cmd);
        if (g_signal)
        {
            shell->exit_status = 130;
            g_signal = 0;
        }
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
            break ;
        }
        if (input[0] != '\0')
            add_history(input);
        process_input(shell, input);
        free(input);
    }
}


int	main(int argc, char **argv, char **envp)
{
    t_shell	*shell;
    char	*input;
    char	prompt[100];

    (void)argc;
    (void)argv;
    shell = init_shell(envp);
    if (!shell)
        return (1);
    setup_signals();
    setup_terminal();
    init_history();  // Initialize history
    ft_display_welcome();
    while (!shell->should_exit)
    {
        create_prompt(prompt, shell->exit_status);
        input = readline(prompt);
        if (!input)
            break ;
        add_to_history(input);  // Add to history
        process_input(shell, input);
        free(input);
    }
    save_history();  // Save history before exit
    free_shell(shell);
    return (shell->exit_status);
}
