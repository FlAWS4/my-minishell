/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:34:56 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:49:02 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create a new environment variable node
 */
t_env	*create_env_node(char *key, char *value)
{
    t_env	*new_node;

    new_node = malloc(sizeof(t_env));
    if (!new_node)
        return (NULL);
    new_node->key = ft_strdup(key);
    if (!new_node->key)
    {
        free(new_node);
        return (NULL);
    }
    new_node->value = ft_strdup(value);
    if (!new_node->value)
    {
        free(new_node->key);
        free(new_node);
        return (NULL);
    }
    new_node->next = NULL;
    return (new_node);
}

/**
 * Add environment variable to list
 */
void	add_env_var(t_env **env_list, t_env *new_node)
{
    t_env	*current;

    if (!*env_list)
    {
        *env_list = new_node;
        return ;
    }
    current = *env_list;
    while (current->next)
        current = current->next;
    current->next = new_node;
}

/**
 * Split environment string into key and value
 */
void	split_env_string(char *str, char **key, char **value)
{
    int	i;
    int	len;

    i = 0;
    *key = NULL;
    *value = NULL;
    while (str[i] && str[i] != '=')
        i++;
    *key = malloc(i + 1);
    if (!*key)
        return ;
    len = i;
    i = 0;
    while (i < len)
    {
        (*key)[i] = str[i];
        i++;
    }
    (*key)[i] = '\0';
    if (str[len] == '=')
        *value = ft_strdup(&str[len + 1]);
    else
        *value = ft_strdup("");
    if (!*value)
    {
        free(*key);
        *key = NULL;
    }
}

/**
 * Initialize environment variables from envp
 */
t_env	*init_env(char **envp)
{
    t_env	*env_list;
    t_env	*new_node;
    char	*key;
    char	*value;
    int		i;

    env_list = NULL;
    i = 0;
    while (envp[i])
    {
        split_env_string(envp[i], &key, &value);
        if (key && value)
        {
            new_node = create_env_node(key, value);
            if (new_node)
                add_env_var(&env_list, new_node);
        }
        if (key)
            free(key);
        if (value)
            free(value);
        i++;
    }
    return (env_list);
}

/**
 * Get environment variable value by key
 */
char	*get_env_value(t_env *env_list, const char *key)
{
    t_env	*current;
    char	*value;

    if (!env_list || !key)
        return (NULL);
    current = env_list;
    while (current)
    {
        if (ft_strcmp(current->key, key) == 0)
        {
            value = ft_strdup(current->value);
            return (value);
        }
        current = current->next;
    }
    return (NULL);
}
