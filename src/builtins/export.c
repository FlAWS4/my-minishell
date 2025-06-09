/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:51 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/09 23:41:30 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Compare two environment variables by key name
 * Used for sorting environment variables alphabetically
 */
static int env_var_compare(const void *a, const void *b)
{
    t_env *env_a = *(t_env **)a;
    t_env *env_b = *(t_env **)b;
    
    return ft_strcmp(env_a->key, env_b->key);
}

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
        
    i = 0;
    while (value[i])
    {
        if (value[i] == '\"' || value[i] == '\\' || value[i] == '$')
            ft_putchar_fd('\\', 1);
        ft_putchar_fd(value[i], 1);
        i++;
    }
}

/**
 * Print a single environment variable in export format
 * Shows all exported variables, whether they're in env or not
 */
static void print_env_var(t_env *env)
{
    if (!env || !env->key || !env->exported)
        return;
        
    ft_putstr_fd("declare -x ", 1);
    ft_putstr_fd(env->key, 1);
    
    // Only print =value part if it's in environment (has a value with = sign)
    if (env->in_env)
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
 * Process a single export argument
 * Handles variable assignment with proper error messages
 */
static int process_export_arg(t_shell *shell, char *arg)
{
    char *equals_pos;
    char *key;
    int result;
    
    if (!arg || !*arg)
        return (1);
    
    // Find equals sign
    equals_pos = ft_strchr(arg, '=');
    
    if (equals_pos)
    {
        // We have an equals sign - split string
        *equals_pos = '\0';  // Temporarily split the string
        key = arg;
        
        // Check if key is valid
        if (!is_valid_identifier(key))
        {
            ft_putstr_fd("minishell: export: `", 2);
            ft_putstr_fd(arg, 2);
            ft_putstr_fd("': not a valid identifier\n", 2);
            *equals_pos = '=';  // Restore the string
            return (1);
        }
        
        // Set in environment (this adds to env output)
        result = set_env_var(&shell->env, key, equals_pos + 1);
        *equals_pos = '=';  // Restore the string
        return (!result);
    }
    else
    {
        // No equals sign - only mark for export if valid
        if (!is_valid_identifier(arg))
        {
            ft_putstr_fd("minishell: export: `", 2);
            ft_putstr_fd(arg, 2);
            ft_putstr_fd("': not a valid identifier\n", 2);
            return (1);
        }
        
        // Mark for export but don't add to environment
        result = mark_var_for_export(&shell->env, arg);
        return (!result);
    }
}

/**
 * Built-in export command
 * With no arguments, displays all exported variables
 * With arguments, adds/updates environment variables
 */
int builtin_export(t_shell *shell, t_cmd *cmd)
{
    int i;
    int status;

    if (!shell || !cmd || !cmd->args)
        return (1);
        
    // With no arguments, display current environment
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
