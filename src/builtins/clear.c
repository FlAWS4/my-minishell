/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clear.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:56:04 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/19 21:59:35 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Built-in clear command
 * Clears the terminal screen
 */
int	builtin_clear(void)
{
    ft_putstr_fd("\033[H\033[2J", 1);
    return (0);
}
