/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:51 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/19 20:47:36 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if variable name is valid
 */
static int	is_valid_var_name(char *name)
{
    int	i;

    if (!name || !*name)
        return (0);
    if (!(name[0] == '_' || (name[0] >= 'a' && name[0] <= 'z')
            || (name[0] >= 'A' && name[0] <= 'Z')))
        return (0);
    i = 1;
    while (name[i] && name[i] != '=')
    {
        if (!(name[i] == '_' || (name[i] >= 'a' && name[i] <= 'z')
                || (name[i] >= 'A' && name[i] <= 'Z')
                || (name[i] >= '0' && name[i] <= '9')))
            return (0);
        i++;
    }
    return (1);
}

/**
 * Update or add environment variable
 */
static void	update_env_var(t_shell *shell, char *key, char *value)
{
    t_env	*env;
    t_env	*new_node;

    env = shell->env;
    while (env)
    {
        if (ft_strcmp(env->key, key) == 0)
        {
            free(env->value);
            env->value = value ? ft_strdup(value) : ft_strdup("");
            free(key);
            return ;
        }
        env = env->next;
    }
    new_node = create_env_node(key, value ? value : "");
    add_env_var(&shell->env, new_node);
    free(key);
}

/**
 * Process a single export argument
 */
static int	process_export_arg(t_shell *shell, char *arg)
{
    char	*key;
    char	*value;
    int		i;

    i = 0;
    while (arg[i] && arg[i] != '=')
        i++;
    key = ft_substr(arg, 0, i);
    if (!is_valid_var_name(key))
    {
        ft_putstr_fd("minishell: export: `", 2);
        ft_putstr_fd(arg, 2);
        ft_putstr_fd("': not a valid identifier\n", 2);
        free(key);
        return (1);
    }
    value = (arg[i] == '=') ? ft_strdup(&arg[i + 1]) : NULL;
    update_env_var(shell, key, value);
    free(value);
    return (0);
}

/**
 * Built-in export command
 */
int	builtin_export(t_shell *shell, t_cmd *cmd)
{
    int	i;
    int	status;

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
