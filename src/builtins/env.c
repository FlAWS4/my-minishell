/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:46 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/16 02:56:02 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Setup node after initial creation
 */
static void	setup_env_node(t_env *node, int exported, int in_env)
{
    node->exported = exported;
    node->in_env = in_env;
    node->next = NULL;
}
/**
 * Handle SHLVL environment variable properly
 */
static void increment_shlvl(t_env **env_list)
{
    t_env *shlvl_var;
    int level;
    char *new_value;
    
    shlvl_var = find_env_var(*env_list, "SHLVL");
    if (!shlvl_var)
    {
        // SHLVL not found, create with value "1"
        set_env_var(env_list, "SHLVL", "1");
        return;
    }
    
    // Get current value
    if (!shlvl_var->value || !*shlvl_var->value)
        level = 0;
    else
        level = ft_atoi(shlvl_var->value);
    
    // Handle invalid values
    if (level < 0)
        level = 0;
    else if (level >= 999)
    {
        // Bash warns at 1000 and sets to 1
        if (level == 999)
            display_shlvl_warning(1000);
        level = 0;
    }
    
    // Increment level
    level++;
    
    // Convert back to string
    new_value = ft_itoa(level);
    if (!new_value)
    {
        display_error(ERROR_MEMORY, "SHLVL", "Memory allocation failed");
        return;
    }
    
    // Update environment
    free(shlvl_var->value);
    shlvl_var->value = new_value;
}

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
    setup_env_node(new_node, 0, 0);
    return (new_node);
}

/**
 * Add environment variable to list
 */
void	add_env_var(t_env **env_list, t_env *new_node)
{
    t_env	*current;

    if (!env_list || !new_node)
        return ;
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
 * Find environment variable by key
 */
t_env	*find_env_var(t_env *env_list, const char *key)
{
    t_env	*current;

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
 * Check if a string is a valid shell variable identifier
 */
int is_valid_identifier(char *id)
{
    int i;
    
    if (!id || !*id) // Check if the string is NULL or empty
        return (0);
    
    // First character must be letter or underscore
    if (!ft_isalpha(id[0]) && id[0] != '_')
        return (0);
    
    // Rest can be letters, numbers, or underscores
    i = 1;
    while (id[i])
    {
        if (!ft_isalnum(id[i]) && id[i] != '_')
            return (0);
        i++;
    }
    return (1);
}

/**
 * Create new export node when variable doesn't exist
 */
static int	create_export_node(t_env **env_list, char *key)
{
    t_env	*new_node;

    new_node = create_env_node(key, NULL);
    if (!new_node)
    {
        display_error(ERROR_EXPORT, key, "Memory allocation failed");
        return (0);
    }
    new_node->exported = 1;
    new_node->in_env = 0;
    add_env_var(env_list, new_node);
    return (1);
}

/**
 * Mark a variable for export without adding it to environment
 */
int	mark_var_for_export(t_env **env_list, char *key)
{
    t_env	*existing;

    if (!env_list || !key)
        return (0);
    existing = find_env_var(*env_list, key);
    if (existing)
    {
        existing->exported = 1;
        return (1);
    }
    else
        return (create_export_node(env_list, key));
}

/**
 * Update existing environment variable
 */
static int	update_env_var(t_env *existing, char *value)
{
    free(existing->value);
    if (value)
        existing->value = ft_strdup(value);
    else
        existing->value = ft_strdup("");
    existing->exported = 1;
    existing->in_env = 1;
    return (existing->value != NULL);
}

/**
 * Create a new environment variable
 */
static int	create_new_env_var(t_env **env_list, char *key, char *value)
{
    t_env	*new_node;

    new_node = create_env_node(key, value);
    if (!new_node)
    {
        display_error(ERROR_ENV, key, "Memory allocation failed");
        return (0);
    }
    new_node->exported = 1;
    new_node->in_env = 1;
    add_env_var(env_list, new_node);
    return (1);
}

/**
 * Update or add environment variable
 */
int	set_env_var(t_env **env_list, char *key, char *value)
{
    t_env	*existing;

    if (!env_list || !key)
        return (0);
    existing = find_env_var(*env_list, key);
    if (existing)
        return (update_env_var(existing, value));
    else
        return (create_new_env_var(env_list, key, value));
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
        return ;
    while (str[i] && str[i] != '=')
        i++;
    *key = ft_substr(str, 0, i);
    if (!*key)
        return ;
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
 * Process a single environment string
 */
static void process_env_string(char *env_str, t_env **env_list)
{
    char *key;
    char *value;
    t_env *new_node;

    split_env_string(env_str, &key, &value);
    if (key && value)
    {
        new_node = create_env_node(key, value);
        if (new_node)
        {
            new_node->exported = 1;
            new_node->in_env = 1;
            add_env_var(env_list, new_node);
        }
        else
            display_error(ERROR_ENV, key, "Failed to create environment variable");
    }
    if (key)
        free(key);
    if (value)
        free(value);
}

/**
 * Initialize environment variables from envp
 */
t_env *init_env(char **envp)
{
    t_env *env_list;
    int i;

    env_list = NULL;
    if (!envp)
        return (NULL);
    i = 0;
    while (envp[i])
    {
        process_env_string(envp[i], &env_list);
        i++;
    }
    
    // Add SHLVL handling
    increment_shlvl(&env_list);
    
    return (env_list);
}

/**
 * Get environment variable value by key
 * Note: Caller must free the returned string
 */
char *get_env_value(t_env *env_list, const char *key)
{
    t_env *current;

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
    t_env	*current;
    t_env	*prev;

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
 * Count environment variables for array creation
 */
static int	count_env_for_array(t_env *env_list)
{
    int		count;
    t_env	*current;

    count = 0;
    current = env_list;
    while (current)
    {
        if (current->in_env)
            count++;
        current = current->next;
    }
    return (count);
}

/**
 * Create single environment string (KEY=VALUE)
 */
static char *create_env_string(t_env *env)
{
    char *temp;
    char *result;
    char *value_to_use;

    temp = ft_strjoin(env->key, "=");
    if (!temp)
        return (NULL);
    
    // Replace ternary with explicit if/else
    if (env->value)
        value_to_use = env->value;
    else
        value_to_use = "";
    
    result = ft_strjoin(temp, value_to_use);
    free(temp);
    return (result);
}


/**
 * Handle error in env array creation
 */
static char	**handle_env_array_error(char **env_array, int i)
{
    while (--i >= 0)
        free(env_array[i]);
    free(env_array);
    return (NULL);
}

/**
 * Fill environment array with string values
 */
static char	**fill_env_array(t_env *env_list, char **env_array, int count)
{
    t_env	*current;
    int		i;

    i = 0;
    current = env_list;
    while (current && i < count)
    {
        if (current->in_env)
        {
            env_array[i] = create_env_string(current);
            if (!env_array[i])
                return (handle_env_array_error(env_array, i));
            i++;
        }
        current = current->next;
    }
    env_array[i] = NULL;
    return (env_array);
}

/**
 * Convert environment linked list to string array (for execve)
 */
char	**env_to_array(t_env *env_list)
{
    char	**env_array;
    int		count;

    if (!env_list)
        return (NULL);
    count = count_env_for_array(env_list);
    env_array = (char **)malloc(sizeof(char *) * (count + 1));
    if (!env_array)
        return (NULL);
    return (fill_env_array(env_list, env_array, count));
}

/**
 * Free the environment array created by env_to_array
 */
void	free_env_array(char **env_array)
{
    int	i;

    if (!env_array)
        return ;
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
int	count_env_vars(t_env *env)
{
    int	count;

    count = 0;
    while (env)
    {
        count++;
        env = env->next;
    }
    return (count);
}

/**
 * Display environment variables
 */
static void	display_env_vars(t_env *env)
{
    while (env)
    {
        if (env->key && env->in_env)
        {
            ft_putstr_fd(env->key, STDOUT_FILENO);
            ft_putchar_fd('=', STDOUT_FILENO);
            if (env->value)
                ft_putstr_fd(env->value, STDOUT_FILENO);
            ft_putchar_fd('\n', STDOUT_FILENO);
        }
        env = env->next;
    }
}

/**
 * Built-in env command
 */
int	builtin_env(t_shell *shell)
{
    if (!shell)
        return (1);
    if (shell->cmd && shell->cmd->args && shell->cmd->args[1])
    {
        display_error(ERROR_ENV, shell->cmd->args[1], "too many arguments");
        return (1);
    }
    if (shell->env)
        display_env_vars(shell->env);
    return (0);
}
