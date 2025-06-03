/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 03:39:11 by my42             ###   ########.fr       */
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
        if (cmd->redirections)
            free_redirection_list(cmd->redirections);
            
        // Close any open file descriptors
        if (cmd->input_fd >= 0)
            close(cmd->input_fd);
        if (cmd->output_fd >= 0)
            close(cmd->output_fd);
            
        tmp = cmd;
        cmd = cmd->next;
        free(tmp);
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
