/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 18:02:37 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/23 22:32:22 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void process_cmd_status(t_shell *shell, int status)
{
    // Initialize BEFORE any conditional logic
    shell->exit_status = 1;
    
    // Explicitly check each bit field before using them
    if (WIFEXITED(status))
    {
        shell->exit_status = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        if (sig == SIGINT)
        {
            ft_putstr_fd("\n", 1);
            shell->exit_status = 130;
        }
        else if (sig == SIGQUIT)
        {
            ft_putstr_fd("Quit (core dumped)\n", 1);
            shell->exit_status = 131;
        }
        else
            shell->exit_status = 128 + sig;
    }
    // Default already set at the top
}

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

/**
 * Copy string section to result at given position
 */
static int copy_str_section(char *result, char *str, int pos, int len)
{
    int i;
    
    i = 0;
    while (i < len)
    {
        result[pos + i] = str[i];
        i++;
    }
    return (pos + i);
}

/**
 * Build environment string manually without using library functions
 */
static char *build_env_string(t_env *env)
{
    char *result;
    int key_len;
    int val_len;
    int pos;
    
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
 * Convert environment to string array
 */
char **env_to_array(t_env *env)
{
    int     count;
    t_env   *temp;
    char    **array;
    int     i;
    
    // Count vars
    count = 0;
    temp = env;
    while (temp && ++count)
        temp = temp->next;
        
    array = malloc(sizeof(char *) * (count + 1));
    if (!array)
        return (NULL);
        
    // Fill array
    temp = env;
    i = 0;
    while (temp)
    {
        array[i] = build_env_string(temp);
        temp = temp->next;
        i++;
    }
    array[i] = NULL;
    return (array);
}
