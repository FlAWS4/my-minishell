/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:51 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 16:50:37 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if variable name is valid
 * Name must start with letter or underscore and contain only alphanumerics
 */
int	is_valid_var_name(char *name)
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
 * Handles memory for key and value
 */
static int	update_env_var(t_shell *shell, char *key, char *value)
{
    t_env	*env;
    t_env	*new_node;

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
}

/**
 * Process a single export argument
 * Handles variable assignment with proper error messages
 */
static int	process_export_arg(t_shell *shell, char *arg)
{
    char	*key;
    char	*value;
    int		i;

    if (!arg || !*arg)
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