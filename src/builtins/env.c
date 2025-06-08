/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:46 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 21:08:01 by my42             ###   ########.fr       */
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

/**
 * Convert environment linked list to string array (for execve)
 */
char **env_to_array(t_env *env_list)
{
    t_env *current;
    char **env_array;
    char *temp;
    int count;
    int i;
    
    if (!env_list)
        return (NULL);
    
    // Count environment variables
    count = 0;
    current = env_list;
    while (current)
    {
        count++;
        current = current->next;
    }
    
    // Allocate array for strings plus NULL terminator
    env_array = (char **)malloc(sizeof(char *) * (count + 1));
    if (!env_array)
        return (NULL);
    
    // Fill array with environment strings
    i = 0;
    current = env_list;
    while (current)
    {
        // Create "KEY=VALUE" string
        temp = ft_strjoin(current->key, "=");
        if (!temp)
        {
            // Free previously allocated strings
            while (--i >= 0)
                free(env_array[i]);
            free(env_array);
            return (NULL);
        }
        
        env_array[i] = ft_strjoin(temp, current->value);
        free(temp);
        
        if (!env_array[i])
        {
            // Free previously allocated strings
            while (--i >= 0)
                free(env_array[i]);
            free(env_array);
            return (NULL);
        }
        
        i++;
        current = current->next;
    }
    
    // NULL terminate the array
    env_array[i] = NULL;
    return (env_array);
}

/**
 * Free the environment array created by env_to_array
 */
void free_env_array(char **env_array)
{
    int i;
    
    if (!env_array)
        return;
    
    i = 0;
    while (env_array[i])
    {
        free(env_array[i]);
        i++;
    }
    
    free(env_array);
}
/**
 * Count number of environment variables
 */
int count_env_vars(t_env *env)
{
    int count;
    
    count = 0;
    while (env)
    {
        count++;
        env = env->next;
    }
    
    return (count);
}
/**
 * Built-in env command
 * Displays all environment variables in format KEY=VALUE
 * Does not accept arguments (will print error if any are provided)
 */
int	builtin_env(t_shell *shell)
{
    t_env	*env;

    if (!shell)
        return (1);
        
    // Check for arguments (env doesn't accept any)
    if (shell->cmd && shell->cmd->args && shell->cmd->args[1])
    {
        print_error("env", "too many arguments");
        return (1);
    }
    
    env = shell->env;
    if (!env)
        return (0);  // Empty environment is valid
        
    while (env)
    {
        if (env->key)
        {
            ft_putstr_fd(env->key, STDOUT_FILENO);
            ft_putchar_fd('=', STDOUT_FILENO);
            
            // Handle potentially NULL values
            if (env->value)
                ft_putstr_fd(env->value, STDOUT_FILENO);
                
            ft_putchar_fd('\n', STDOUT_FILENO);
        }
        env = env->next;
    }
    return (0);
}