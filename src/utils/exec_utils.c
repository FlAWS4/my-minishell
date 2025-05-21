/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 18:02:37 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/21 18:02:49 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

#include "minishell.h"

/**
 * Create a full path by joining directory and command
 */
char *create_path(char *dir, char *cmd)
{
    char *path;
    int len;

    len = ft_strlen(dir) + ft_strlen(cmd) + 2; // +2 for '/' and null terminator
    path = malloc(len);
    if (!path)
        return (NULL);
    
    ft_strlcpy(path, dir, len);
    ft_strlcat(path, "/", len);
    ft_strlcat(path, cmd, len);
    
    return (path);
}