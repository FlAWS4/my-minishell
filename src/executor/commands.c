/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:22:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 06:07:22 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if command is an absolute or relative path
 */
static char	*check_direct_path(char *cmd)
{
    if (cmd[0] == '/' || cmd[0] == '.')
        return (ft_strdup(cmd));
    return (NULL);
}

/**
 * Search for command in PATH directories
 */
static char	*search_in_path(char **paths, char *cmd)
{
    char	*full_path;
    int		i;
    
    i = 0;
    while (paths && paths[i])
    {
        full_path = create_path(paths[i], cmd);
        if (full_path && access(full_path, X_OK) == 0)
            return (full_path);
        free(full_path);
        i++;
    }
    return (NULL);
}

/**
 * Find command in PATH
 */
char *find_command(t_shell *shell, char *cmd)
{
    char *path_env;
    char **paths;
    char *result;
    
    if (!cmd || !*cmd)
        return (NULL);
    
    // Handle commands with special characters - they're never valid
    int i = 0;
    while (cmd[i])
    {
        if (!ft_isalnum(cmd[i]) && cmd[i] != '/' && cmd[i] != '.' && cmd[i] != '_' && cmd[i] != '-')
            return (NULL);
        i++;
    }
    
    result = check_direct_path(cmd);
    if (result)
        return (result);
    
    path_env = get_env_value(shell->env, "PATH");
    if (!path_env)
        return (NULL);
        
    paths = ft_split(path_env, ':');
    free(path_env);
    
    if (!paths)
        return (NULL);
        
    result = search_in_path(paths, cmd);
    free_str_array(paths);
    return (result);
}

