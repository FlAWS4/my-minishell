/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:55:39 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 20:52:51 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Output integer to file descriptor
 * Handles negative numbers correctly
 */
void	ft_putnbr_fd(int n, int fd)
{
    unsigned int	nb;

    if (fd < 0)
        return;
        
    if (n < 0)
    {
        ft_putchar_fd('-', fd);
        nb = -n;
    }
    else
        nb = (unsigned int)n;
        
    if (nb >= 10)
        ft_putnbr_fd(nb / 10, fd);
        
    ft_putchar_fd((char)(nb % 10 + '0'), fd);
}


