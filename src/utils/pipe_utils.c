/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 21:37:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/23 21:38:13 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process child termination status
 */
static int process_child_status(int status)
{
    int last_status;

    if (WIFEXITED(status))
        last_status = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
    {
        if (WTERMSIG(status) == SIGINT)
        {
            ft_putstr_fd("\n", 1);
            last_status = 130;
        }
        else if (WTERMSIG(status) == SIGQUIT)
        {
            ft_putstr_fd("Quit (core dumped)\n", 1);
            last_status = 131;
        }
        else
            last_status = 128 + WTERMSIG(status);
    }
    else
        last_status = 1;
    return (last_status);
}

/**
 * Wait for all child processes in a pipeline
 */
int wait_for_children(t_shell *shell)
{
    int status;
    int last_status;
    pid_t pid;

    last_status = 0;
    while ((pid = wait(&status)) > 0)
        last_status = process_child_status(status);
    shell->exit_status = last_status;
    return (last_status);
}