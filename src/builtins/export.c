/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:51 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/11 00:25:07 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Compare two environment variables by key name
 * Used for sorting environment variables alphabetically
 */
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
{
    int	i;

    if (!value)
        return ;
    i = 0;
    while (value[i])
    {
        if (value[i] == '\"' || value[i] == '\\' || value[i] == '$')
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
    return (1);
}

/**
 * Process value append for export (used by process_append_op)
 */
static int	append_value(t_shell *shell, char *key, char *value)
{
    t_env	*existing;
    char	*old_value;
    char	*new_value;
    int		result;

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
    if (!cmd->args[1])
    {
        print_sorted_env(shell);
        return (0);
    }
    status = 0;
    i = 1;
    while (cmd->args[i])
    {
        if (process_export_arg(shell, cmd->args[i]) != 0)
            status = 1;
        i++;
    }
    return (status);
}
