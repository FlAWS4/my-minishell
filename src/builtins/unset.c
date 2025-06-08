/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:34:36 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 01:57:11 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


/**
 * Free environment node
 * Handles NULL pointers safely
 */
static void	free_env_node(t_env *node)
{
    if (node)
    {
        if (node->key)
            free(node->key);
        if (node->value)
            free(node->value);
        free(node);
    }
}

/**
 * Remove variable from environment
 * Handles list manipulation and memory cleanup
 */
static void	remove_env_var(t_shell *shell, char *key)
{
    t_env	*curr;
    t_env	*prev;

    if (!shell || !key)
        return;
        
    curr = shell->env;
    prev = NULL;
    
    while (curr)
    {
        if (ft_strcmp(curr->key, key) == 0)
        {
            if (prev)
                prev->next = curr->next;
            else
                shell->env = curr->next;
            free_env_node(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

/**
 * Built-in unset command
 * Removes variables from the environment
 */
int	builtin_unset(t_shell *shell, t_cmd *cmd)
{
    int	i;
    int	status;

    if (!shell || !cmd || !cmd->args)
        return (1);
        
    status = 0;
    i = 1;
    
    // Handle case of no arguments (success, do nothing)
    if (!cmd->args[i])
        return (0);
        
    while (cmd->args[i])
    {
        if (!is_valid_var_name(cmd->args[i]))
        {
            ft_putstr_fd("minishell: unset: `", STDERR_FILENO);
            ft_putstr_fd(cmd->args[i], STDERR_FILENO);
            ft_putstr_fd("': not a valid identifier\n", STDERR_FILENO);
            status = 1;
        }
        else
            remove_env_var(shell, cmd->args[i]);
        i++;
    }
    
    return (status);
}
