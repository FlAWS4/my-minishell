/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:34:56 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 16:59:02 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create a new environment variable node
 */
t_env	*create_env_node(char *key, char *value)
{
    t_env	*new_node;

    if (!key)
        return (NULL);
    new_node = malloc(sizeof(t_env));
    if (!new_node)
        return (NULL);
    new_node->key = ft_strdup(key);
    if (!new_node->key)
    {
        free(new_node);
        return (NULL);
    }
    if (value)
        new_node->value = ft_strdup(value);
    else
        new_node->value = ft_strdup("");
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

    if (!env_list || !new_node)
        return;
    if (!*env_list)
    {
        *env_list = new_node;
        return;
    }
    current = *env_list;
    while (current->next)
        current = current->next;
    current->next = new_node;
}

/**
 * Find environment variable by key
 */
t_env *find_env_var(t_env *env_list, const char *key)
{
    t_env *current;
    
    if (!env_list || !key)
        return (NULL);
    current = env_list;
    while (current)
    {
        if (ft_strcmp(current->key, key) == 0)
            return (current);
        current = current->next;
    }
    return (NULL);
}

/**
 * Update or add environment variable
 */
int set_env_var(t_env **env_list, char *key, char *value)
{
    t_env *existing;
    t_env *new_node;
    
    if (!env_list || !key)
        return (0);
    
    existing = find_env_var(*env_list, key);
    if (existing)
    {
        // Update existing variable
        free(existing->value);
        if (value)
            existing->value = ft_strdup(value);
        else
            existing->value = ft_strdup("");
        return (existing->value != NULL);
    }
    else
    {
        // Add new variable
        new_node = create_env_node(key, value);
        if (!new_node)
            return (0);
        add_env_var(env_list, new_node);
        return (1);
    }
}

/**
 * Split environment string into key and value
 */
void	split_env_string(char *str, char **key, char **value)
{
    int	i;

    i = 0;
    *key = NULL;
    *value = NULL;
    
    if (!str)
        return;
        
    while (str[i] && str[i] != '=')
        i++;
        
    *key = ft_substr(str, 0, i);
    if (!*key)
        return;
        
    if (str[i] == '=')
        *value = ft_strdup(&str[i + 1]);
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
    if (!envp)
        return (NULL);
        
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

    if (!env_list || !key)
        return (NULL);
        
    current = env_list;
    while (current)
    {
        if (ft_strcmp(current->key, key) == 0)
            return (ft_strdup(current->value));
        current = current->next;
    }
    return (NULL);
}

/**
 * Delete environment variable by key
 */
int	delete_env_var(t_env **env_list, char *key)
{
    t_env *current;
    t_env *prev;
    
    if (!env_list || !*env_list || !key)
        return (0);
        
    current = *env_list;
    prev = NULL;
    
    while (current)
    {
        if (ft_strcmp(current->key, key) == 0)
        {
            if (prev)
                prev->next = current->next;
            else
                *env_list = current->next;
                
            free(current->key);
            free(current->value);
            free(current);
            return (1);
        }
        prev = current;
        current = current->next;
    }
    return (0);
}
