/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_shell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 22:42:29 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/27 22:50:06 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

/**
 * backup_standard_fds - Save copies of standard file descriptors
 * @shell: Shell structure to store file descriptor copies
 *
 * Duplicates STDIN and STDOUT for restoration after redirections
 * Exits with error if duplication fails
 */
void	init_shell_fds(t_shell *shell)
{
    shell->saved_stdin = dup(STDIN_FILENO);
    shell->saved_stdout = dup(STDOUT_FILENO);
    if (shell->saved_stdin == -1 || shell->saved_stdout == -1)
    {
        error("dup", NULL, strerror(errno));
        clean_and_exit_shell(shell, 1);
    }
}
