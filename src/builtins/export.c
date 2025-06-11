/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:51 by mshariar          #+#    #+#             */
<<<<<<< HEAD
/*   Updated: 2025/06/11 00:25:07 by mshariar         ###   ########.fr       */
=======
/*   Updated: 2025/06/03 20:15:22 by my42             ###   ########.fr       */
>>>>>>> main
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Compare two environment variables by key name
 * Used for sorting environment variables alphabetically
 */
<<<<<<< HEAD
static int	env_var_compare(const void *a, const void *b)
{
    t_env	*env_a;
    t_env	*env_b;

    env_a = *(t_env **)a;
    env_b = *(t_env **)b;
    return (ft_strcmp(env_a->key, env_b->key));
}

/**
 * Sort environment variables using qsort for better performance
 */
static void	sort_env_array(t_env **env_array, int count)
{
    if (!env_array || count <= 0)
        return ;
    qsort(env_array, count, sizeof(t_env *), env_var_compare);
}

/**
 * Print value with proper escaping of quotes
 */
static void	print_escaped_value(char *value)
=======
static int env_var_compare(const void *a, const void *b)
>>>>>>> main
{
    t_env *env_a = *(t_env **)a;
    t_env *env_b = *(t_env **)b;
    
    return ft_strcmp(env_a->key, env_b->key);
}

<<<<<<< HEAD
    if (!value)
        return ;
=======
/**
 * Sort environment variables using qsort for better performance
 */
static void sort_env_array(t_env **env_array, int count)
{
    if (!env_array || count <= 0)
        return;
        
    qsort(env_array, count, sizeof(t_env *), env_var_compare);
}

/**
 * Print value with proper escaping of quotes
 */
static void print_escaped_value(char *value)
{
    int i;
    
    if (!value)
        return;
        
>>>>>>> main
    i = 0;
    while (value[i])
    {
        if (value[i] == '\"' || value[i] == '\\' || value[i] == '$')
<<<<<<< HEAD
            ft_putchar_fd('\\', STDOUT_FILENO);
        ft_putchar_fd(value[i], STDOUT_FILENO);
        i++;
    }
}

/**
 * Print a single environment variable in export format
 * Shows all exported variables, whether they're in env or not
 */
static void	print_env_var(t_env *env)
{
    if (!env || !env->key || !env->exported)
        return ;
    ft_putstr_fd("declare -x ", STDOUT_FILENO);
    ft_putstr_fd(env->key, STDOUT_FILENO);
    if (env->in_env)
    {
        ft_putstr_fd("=\"", STDOUT_FILENO);
        print_escaped_value(env->value);
        ft_putstr_fd("\"", STDOUT_FILENO);
    }
    ft_putstr_fd("\n", STDOUT_FILENO);
}

/**
 * Allocate array for sorting environment variables
 */
static t_env	**alloc_env_array(t_shell *shell, int count)
{
    t_env	**env_array;

    (void)shell; // Unused parameter, but needed for function signature
    env_array = malloc(sizeof(t_env *) * count);
    if (!env_array)
    {
        display_error(ERROR_MEMORY, "export", "Memory allocation failed");
        return (NULL);
    }
    return (env_array);
}

/**
 * Copy environment variables to array for sorting
 */
static void	copy_env_to_array(t_shell *shell, t_env **env_array)
{
    t_env	*curr;
    int		i;

    i = 0;
    curr = shell->env;
    while (curr)
    {
        env_array[i++] = curr;
        curr = curr->next;
    }
}

/**
 * Print environment variables in sorted order
 */
void	print_sorted_env(t_shell *shell)
{
    t_env	**env_array;
    int		count;
    int		i;

    if (!shell || !shell->env)
        return ;
    count = count_env_vars(shell->env);
    if (count <= 0)
        return ;
    env_array = alloc_env_array(shell, count);
    if (!env_array)
        return ;
    copy_env_to_array(shell, env_array);
    sort_env_array(env_array, count);
    i = 0;
    while (i < count)
        print_env_var(env_array[i++]);
    free(env_array);
}
/**
 * Handle invalid identifier error
 */
static int	handle_invalid_id(char *arg)
{
    display_error(ERROR_EXPORT, arg, "not a valid identifier");
=======
            ft_putchar_fd('\\', 1);
        ft_putchar_fd(value[i], 1);
        i++;
    }
}

/**
 * Print a single environment variable in export format
 */
static void print_env_var(t_env *env)
{
    if (!env || !env->key)
        return;
        
    ft_putstr_fd("declare -x ", 1);
    ft_putstr_fd(env->key, 1);
    
    // Only print =value part if value exists
    if (env->value && env->value[0] != '\0')
    {
        ft_putstr_fd("=\"", 1);
        print_escaped_value(env->value);
        ft_putstr_fd("\"", 1);
    }
    ft_putstr_fd("\n", 1);
}

/**
 * Print environment variables in sorted order
 */
void print_sorted_env(t_shell *shell)
{
    t_env **env_array;
    t_env *curr;
    int count;
    int i;

    if (!shell || !shell->env)
        return;
        
    // Count environment variables
    count = count_env_vars(shell->env);
    if (count <= 0)
        return;
        
    // Allocate array for sorting
    env_array = malloc(sizeof(t_env *) * count);
    if (!env_array)
    {
        print_error("export", "Memory allocation failed");
        return;
    }
    
    // Copy pointers to array
    i = 0;
    curr = shell->env;
    while (curr && i < count)
    {
        env_array[i++] = curr;
        curr = curr->next;
    }
    
    // Sort and print
    sort_env_array(env_array, count);
    i = 0;
    while (i < count)
        print_env_var(env_array[i++]);
        
    // Cleanup
    free(env_array);
}

/**
 * Check if a variable name is valid for export
 * Variable names must start with a letter or underscore
 * and contain only alphanumeric characters or underscores
 */
int is_valid_var_name(char *name)
{
    int i;
    
    if (!name || !name[0])
        return (0);
        
    // First character must be letter or underscore
    if (!ft_isalpha(name[0]) && name[0] != '_')
        return (0);
        
    // Rest can be alphanumeric or underscore
    i = 1;
    while (name[i])
    {
        if (!ft_isalnum(name[i]) && name[i] != '_')
            return (0);
        i++;
    }
    
>>>>>>> main
    return (1);
}

/**
<<<<<<< HEAD
 * Process value append for export (used by process_append_op)
 */
static int	append_value(t_shell *shell, char *key, char *value)
=======
 * Update or add environment variable
 * Handles memory for key and value
 */
static int	update_env_var(t_shell *shell, char *key, char *value)
>>>>>>> main
{
    t_env	*existing;
    char	*old_value;
    char	*new_value;
    int		result;

<<<<<<< HEAD
    existing = find_env_var(shell->env, key);
    if (existing && existing->value)
    {
        old_value = ft_strdup(existing->value);
        new_value = ft_strjoin(old_value, value);
        free(old_value);
        result = set_env_var(&shell->env, key, new_value);
        free(new_value);
        return (result);
    }
    return (set_env_var(&shell->env, key, value));
}

/**
 * Process append operation (+=) for export
 */
static int	process_append_op(t_shell *shell, char *arg, char *append_pos)
{
    char	*key;
    int		result;

    *append_pos = '\0';
    key = arg;
    if (!is_valid_identifier(key))
    {
        *append_pos = '+';
        return (handle_invalid_id(arg));
    }
    result = append_value(shell, key, append_pos + 2);
    *append_pos = '+';
    return (!result);
}


/**
 * Process regular assignment (=) for export
 */
static int	process_assignment(t_shell *shell, char *arg, char *equals_pos)
{
    char	*key;
    int		result;

    *equals_pos = '\0';
    key = arg;
    if (!is_valid_identifier(key))
    {
        *equals_pos = '=';
        return (handle_invalid_id(arg));
    }
    result = set_env_var(&shell->env, key, equals_pos + 1);
    *equals_pos = '=';
    return (!result);
=======
    if (!shell || !key)
        return (0);
        
    env = shell->env;
    while (env)
    {
        if (ft_strcmp(env->key, key) == 0)
        {
            free(env->value);
            env->value = value ? ft_strdup(value) : ft_strdup("");
            free(key);
            return (1);
        }
        env = env->next;
    }
    
    new_node = create_env_node(key, value ? value : "");
    if (!new_node)
    {
        free(key);
        return (0);
    }
    
    add_env_var(&shell->env, new_node);
    free(key);
    return (1);
>>>>>>> main
}

/**
 * Process a single export argument
 * Handles variable assignment with proper error messages
 */
static int	process_export_arg(t_shell *shell, char *arg)
{
    char	*equals_pos;
    char	*append_pos;

    if (!arg || !*arg)
<<<<<<< HEAD
        return (1);
    append_pos = ft_strstr(arg, "+=");
    if (append_pos)
        return (process_append_op(shell, arg, append_pos));
    equals_pos = ft_strchr(arg, '=');
    if (equals_pos)
        return (process_assignment(shell, arg, equals_pos));
    if (!is_valid_identifier(arg))
        return (handle_invalid_id(arg));
    return (!mark_var_for_export(&shell->env, arg));
=======
        return (1);
        
    i = 0;
    while (arg[i] && arg[i] != '=')
        i++;
        
    key = ft_substr(arg, 0, i);
    if (!key || !is_valid_var_name(key))
    {
        if (key)
            free(key);
        ft_putstr_fd("minishell: export: `", STDERR_FILENO);
        ft_putstr_fd(arg, STDERR_FILENO);
        ft_putstr_fd("': not a valid identifier\n", STDERR_FILENO);
        return (1);
    }
    
    // Value is everything after first equals sign
    value = (arg[i] == '=') ? &arg[i + 1] : NULL;
    if (!update_env_var(shell, key, value))
        return (1);
        
    return (0);
>>>>>>> main
}

/**
 * Built-in export command
 * With no arguments, displays all exported variables
 * With arguments, adds/updates environment variables
 */
int	builtin_export(t_shell *shell, t_cmd *cmd)
{
    int	i;
    int	status;

    if (!shell || !cmd || !cmd->args)
        return (1);
<<<<<<< HEAD
=======
        
    // With no arguments, display current environment
>>>>>>> main
    if (!cmd->args[1])
    {
        print_sorted_env(shell);
        return (0);
    }
    
    // Process each argument
    status = 0;
    i = 1;
    while (cmd->args[i])
    {
        if (process_export_arg(shell, cmd->args[i]) != 0)
            status = 1;  // Remember errors but continue processing
        i++;
    }
    
    return (status);
}
