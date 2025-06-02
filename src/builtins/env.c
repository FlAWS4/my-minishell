/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:46 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 04:32:07 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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
