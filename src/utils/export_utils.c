/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 20:45:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 16:55:17 by my42             ###   ########.fr       */
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
    
    return (1);
}
/**
 * Check if character is alphabetic
 */
int ft_isalpha(int c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}