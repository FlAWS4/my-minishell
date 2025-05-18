/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/18 20:27:04 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Built-in exit command (placeholder)
 */
int	builtin_exit(t_shell *shell, t_cmd *cmd)
{
    (void)shell;
    (void)cmd;
    ft_putstr_fd("exit command not yet fully implemented\n", 2);
    return (0);
}
