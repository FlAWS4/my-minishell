/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 21:37:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 04:05:40 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process child termination status
 * Returns appropriate exit code based on how child process terminated
 */
static int	process_child_status(int status)
{
    int	last_status;

    if (WIFEXITED(status))
        last_status = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
    {
        if (WTERMSIG(status) == SIGINT)
        {
            ft_putstr_fd("\n", 2);
            last_status = 130;
        }
        else if (WTERMSIG(status) == SIGQUIT)
        {
            ft_putstr_fd("Quit (core dumped)\n", 2);
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
 * Updates shell exit status with status of last command in pipeline
 * Returns the last command's exit status
 */
int	wait_for_children(t_shell *shell)
{
    int		status;
    int		last_status;
    pid_t	pid;

    last_status = 0;
    if (!shell)
        return (1);
        
    while (1)
    {
        pid = waitpid(-1, &status, 0);
        if (pid < 0)
        {
            if (errno == ECHILD)  // No more children
                break;
            else
                return (1);  // Error occurred
        }
        else if (pid == 0)  // No children have exited
            break;
            
        last_status = process_child_status(status);
    }
    
    shell->exit_status = last_status;
    return (last_status);
}
