/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:07:50 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:10:56 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

/**
 * Executes commands with signal state management
 * 
 * @param shell  The shell context
 * @return       1 if interrupted, 0 otherwise
 * 
 * Sets a special exit status (999) to mark command execution,
 * executes commands, then handles signals and restores state.
 */
int	safely_execute_command(t_shell *shell)
{
    int	old_exit_status;

    old_exit_status = g_exit_status;
    g_exit_status = 999;
    execute_command_sequence(shell);
    if (g_exit_status == 130)
    {
        restore_standard_fds(shell);
        return (1);
    }
    if (g_exit_status == 999)
        g_exit_status = old_exit_status;
    return (0);
}

/**
 * Disables the display of control characters in terminal
 * 
 * Modifies terminal settings to hide control characters like ^C
 * when signals are received, providing a cleaner user experience.
 */
void	disable_control_char_echo(void)
{
    struct termios	term;

    if (tcgetattr(STDIN_FILENO, &term) == -1)
        return ;
    term.c_lflag &= ~ECHOCTL;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/**
 * Enables the display of control characters in terminal
 * 
 * Modifies terminal settings to show control characters like ^C
 * when signals are received, restoring default behavior.
 */
void	enable_control_char_echo(void)
{
    struct termios	term;

    if (tcgetattr(STDIN_FILENO, &term) == -1)
        return ;
    term.c_lflag |= ECHOCTL;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

