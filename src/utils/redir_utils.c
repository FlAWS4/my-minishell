/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:55:39 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 21:46:18 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Read a line for heredoc input
 */
char *read_heredoc_line(void)
{
    char *line;
    
    ft_putstr_fd("> ", 1);
    line = get_next_line(STDIN_FILENO);
    
    // Check global signal flag
    if (g_signal == SIGINT)
        return (NULL);
        
    if (!line)
        return (NULL);
        
    // Remove newline character
    int len = ft_strlen(line);
    if (len > 0 && line[len - 1] == '\n')
        line[len - 1] = '\0';
        
    return (line);
}

void	ft_putnbr_fd(int n, int fd)
{
	unsigned int	nb;

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

// Add to your utils.c or gnl.c file
void reset_gnl_buffer(void)
{
    static int dummy_fd = -1;
    char *temp;
    
    // Force GNL to reset its static variable by calling with invalid fd
    temp = get_next_line(dummy_fd);
    if (temp)
        free(temp);
}
