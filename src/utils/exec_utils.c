/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 18:02:37 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:54:32 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create a full path by joining directory and command
 */
char	*create_path(char *dir, char *cmd)
{
    char	*path;
    int		len;

    len = ft_strlen(dir) + ft_strlen(cmd) + 2;
    path = malloc(len);
    if (!path)
        return (NULL);
    ft_strlcpy(path, dir, len);
    ft_strlcat(path, "/", len);
    ft_strlcat(path, cmd, len);
    return (path);
}

/**
 * Copy string section to result at given position
 */
static int	copy_str_section(char *result, char *str, int pos, int len)
{
    int	i;

    i = 0;
    while (i < len)
    {
        result[pos + i] = str[i];
        i++;
    }
    return (pos + i);
}

/**
 * Build environment string in KEY=VALUE format
 */
static char	*build_env_string(t_env *env)
{
    char	*result;
    int		key_len;
    int		val_len;
    int		pos;

    key_len = ft_strlen(env->key);
    val_len = ft_strlen(env->value);
    result = malloc(key_len + 1 + val_len + 1);
    if (!result)
        return (NULL);
    pos = copy_str_section(result, env->key, 0, key_len);
    result[pos++] = '=';
    pos = copy_str_section(result, env->value, pos, val_len);
    result[pos] = '\0';
    return (result);
}

/**
 * Counts the number of environment variables
 */
static int	count_env_vars(t_env *env)
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
 * Free the environment array when an error occurs
 */
static void	free_env_array(char **array, int count)
{
    int	i;

    i = 0;
    while (i < count)
    {
        free(array[i]);
        i++;
    }
    free(array);
}

/**
 * Initialize environment array
 */
static char	**init_env_array(t_env *env, int *count)
{
    char	**array;
    
    *count = count_env_vars(env);
    array = malloc(sizeof(char *) * (*count + 1));
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
