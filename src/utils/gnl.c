/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gnl.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:14:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/16 00:48:15 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char *read_from_fd(int fd)
{
    char buffer[4096];
    ssize_t bytes_read = read(fd, buffer, 4095);
    
    if (bytes_read <= 0)
        return NULL;
        
    buffer[bytes_read] = '\0';
    return ft_strdup(buffer);
}