/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 18:02:37 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 17:03:59 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create a full path by joining directory and command
 */
char	*create_path(char *dir, char *cmd)
{
    char	*path;
    char    *with_slash;
    
    if (!dir || !cmd)
        return (NULL);
        
    with_slash = ft_strjoin(dir, "/");
    if (!with_slash)
        return (NULL);
        
    path = ft_strjoin(with_slash, cmd);
    free(with_slash);
    return (path);
}

/*
 * Copy string section to result at given position

static int	copy_str_section(char *result, char *str, int pos, int len)
{
    int	i;

    if (!result || !str || len < 0)
        return (pos);
        
    i = 0;
    while (i < len && str[i])
    {
        result[pos + i] = str[i];
        i++;
    }
    return (pos + i);
}
*/

/**
 * Build environment string in KEY=VALUE format
 */
static char	*build_env_string(t_env *env)
{
    char	*result;
    char    *temp;
    
    if (!env || !env->key)
        return (NULL);
        
    if (!env->value)
        return (ft_strjoin(env->key, "="));
        
    temp = ft_strjoin(env->key, "=");
    if (!temp)
        return (NULL);
        
    result = ft_strjoin(temp, env->value);
    free(temp);
    return (result);
}

/**
 * Counts the number of environment variables
 */
int	count_env_vars(t_env *env)
{
    int		count;
    t_env	*temp;

    count = 0;
    temp = env;
    while (temp)
    {
        count++;
        temp = temp->next;
    }
    return (count);
}

/**
 * Free the environment array and all its strings
 */
void	free_env_array(char **array, int count)
{
    int	i;

    if (!array)
        return;
        
    if (count < 0)
    {
        // If count is negative, array is NULL-terminated
        i = 0;
        while (array[i])
        {
            free(array[i]);
            i++;
        }
    }
    else
    {
        // Free exactly count items
        i = 0;
        while (i < count)
        {
            free(array[i]);
            i++;
        }
    }
    free(array);
}

/**
 * Initialize environment array
 */
static char	**init_env_array(t_env *env, int *count)
{
    char	**array;
    
    if (!env || !count)
        return (NULL);
        
    *count = count_env_vars(env);
    array = malloc(sizeof(char *) * (*count + 1));
    if (!array)
        return (NULL);
    
    // Initialize all entries to NULL for safety
    for (int i = 0; i <= *count; i++)
        array[i] = NULL;
        
    return (array);
}

/**
 * Convert environment linked list to string array for execve
 */
char	**env_to_array(t_env *env)
{
    int		count;
    t_env	*temp;
    char	**array;
    int		i;

    if (!env)
        return (NULL);
        
    array = init_env_array(env, &count);
    if (!array)
        return (NULL);
        
    temp = env;
    i = 0;
    while (temp)
    {
        array[i] = build_env_string(temp);
        if (!array[i])
        {
            free_env_array(array, i);
            return (NULL);
        }
        temp = temp->next;
        i++;
    }
    array[i] = NULL;
    return (array);
}

/**
 * Check if a command exists and is executable in the given path
 */
int is_executable(char *path)
{
    struct stat file_stat;
    
    if (!path)
        return (0);
        
    if (stat(path, &file_stat) == -1)
        return (0);
        
    return (S_ISREG(file_stat.st_mode) && (file_stat.st_mode & S_IXUSR));
}

/**
 * Find a command in the PATH environment
 */
char *find_command_in_path(char *cmd, t_env *env)
{
    char *path_var;
    char **paths;
    char *cmd_path;
    int i;
    
    if (!cmd || !cmd[0] || !env)
        return (NULL);
        
    // Check if cmd is already a path
    if (cmd[0] == '/' || (cmd[0] == '.' && cmd[1] == '/'))
    {
        if (is_executable(cmd))
            return (ft_strdup(cmd));
        return (NULL);
    }
    
    // Get PATH variable
    path_var = get_env_value(env, "PATH");
    if (!path_var)
        return (NULL);
    
    // Split PATH into directories
    paths = ft_split(path_var, ':');
    free(path_var);
    if (!paths)
        return (NULL);
    
    // Check each directory
    i = 0;
    while (paths[i])
    {
        cmd_path = create_path(paths[i], cmd);
        if (cmd_path && is_executable(cmd_path))
        {
            free_env_array(paths, -1);
            return (cmd_path);
        }
        free(cmd_path);
        i++;
    }
    
    free_env_array(paths, -1);
    return (NULL);
}
int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t			i;

	i = 0;
	if (n == 0)
		return (0);
	while (s1[i] && s2[i] == s1[i] && i < n - 1)
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}
