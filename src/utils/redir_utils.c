/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:55:39 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:01:48 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Read a line for heredoc input
 */
char	*read_heredoc_line(void)
{
    char	*line;
    int		len;

    ft_putstr_fd("> ", 1);
    line = get_next_line(STDIN_FILENO);
    if (!line)
        return (NULL);
    len = ft_strlen(line);
    if (len > 0 && line[len - 1] == '\n')
        line[len - 1] = '\0';
    return (line);
}