/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:34:36 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/18 20:27:50 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Built-in unset command (placeholder)
 */
int	builtin_unset(t_shell *shell, t_cmd *cmd)
{
    (void)shell;
    (void)cmd;
    ft_putstr_fd("unset command not yet fully implemented\n", 2);
    return (0);
}
