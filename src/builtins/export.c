/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:51 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 02:07:23 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Compare two environment variables by key name
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
 * Display exported variables in sorted order
 */
static void	display_exported_vars(t_env *env)
{
    t_env	**env_array;
    int		count;
    int		i;

    if (!env)
        return ;
    count = count_env_vars(env);
    if (count <= 0)
        return ;
    env_array = malloc(sizeof(t_env *) * count);
    if (!env_array)
    {
        display_error(ERROR_MEMORY, "export", "Memory allocation failed");
        return ;
    }
    i = 0;
    while (env)
    {
        env_array[i++] = env;
        env = env->next;
    }
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
 * Find the append operator (+=) in a string
 */
static char	*find_append_op(char *str)
{
    int	i;

    if (!str)
        return (NULL);
    i = 0;
    while (str[i])
    {
        if (str[i] == '+' && str[i + 1] == '=')
            return (&str[i]);
        i++;
    }
    return (NULL);
}

/**
 * Process value append for export
 */
static int	append_value(t_shell *shell, char *key, char *value)
{
    t_env	*existing;
    char	*old_value;
    char	*new_value;
    int		result;

    existing = find_env_var(shell->env, key);
    if (!existing || !existing->value)
        return (set_env_var(&shell->env, key, value));
    old_value = ft_strdup(existing->value);
    if (!old_value)
    {
        display_error(ERROR_MEMORY, "export", "Memory allocation failed");
        return (0);
    }
    new_value = ft_strjoin(old_value, value);
    free(old_value);
    if (!new_value)
    {
        display_error(ERROR_MEMORY, "export", "Memory allocation failed");
        return (0);
    }
    result = set_env_var(&shell->env, key, new_value);
    free(new_value);
    return (result);
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
 * Process assignment (=) for export
 */
static int	process_assignment(t_shell *shell, char *arg, char *equals_pos)
{
    char	*key;
    char	*value;
    int		result;

    *equals_pos = '\0';
    key = arg;
    value = equals_pos + 1;
    if (!is_valid_identifier(key))
    {
        *equals_pos = '=';
        return (handle_invalid_id(arg));
    }
    result = set_env_var(&shell->env, key, value);
    *equals_pos = '=';
    return (!result);
}

/**
 * Process export arguments without assignment
 */
static int	process_no_assignment(t_shell *shell, char *arg)
{
    if (!is_valid_identifier(arg))
        return (handle_invalid_id(arg));
    return (!mark_var_for_export(&shell->env, arg));
}

/**
 * Process a single export argument
 */
static int	process_export_arg(t_shell *shell, char *arg)
{
    char	*equals_pos;
    char	*append_pos;

    if (!arg || !*arg)
        return (handle_invalid_id(""));
    append_pos = find_append_op(arg);
    if (append_pos)
        return (process_append_op(shell, arg, append_pos));
    equals_pos = ft_strchr(arg, '=');
    if (equals_pos)
        return (process_assignment(shell, arg, equals_pos));
    return (process_no_assignment(shell, arg));
}

/**
 * Built-in export command
 */
int	builtin_export(t_shell *shell, t_cmd *cmd)
{
    int	i;
    int	result;

    if (!shell || !cmd)
        return (1);
    if (!cmd->args[1])
    {
        display_exported_vars(shell->env);
        return (0);
    }
    result = 0;
    i = 1;
    while (cmd->args[i])
    {
        if (cmd->args[i][0] == '\0')
            result |= handle_invalid_id("");
        else
            result |= process_export_arg(shell, cmd->args[i]);
        i++;
    }
    return (result);
}
