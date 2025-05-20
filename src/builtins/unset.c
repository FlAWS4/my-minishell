/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:34:36 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/19 21:25:36 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if variable name is valid (same as export)
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
    while (name[i])
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
 * Free environment node
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
 */
static void	remove_env_var(t_shell *shell, char *key)
{
    t_env	*curr;
    t_env	*prev;

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
            return ;
        }
        prev = curr;
        curr = curr->next;
    }
}

/**
 * Built-in unset command
 */
int	builtin_unset(t_shell *shell, t_cmd *cmd)
{
    int	i;
    int	status;

    status = 0;
    i = 1;
    while (cmd->args[i])
    {
        if (!is_valid_var_name(cmd->args[i]))
        {
            ft_putstr_fd("minishell: unset: `", 2);
            ft_putstr_fd(cmd->args[i], 2);
            ft_putstr_fd("': not a valid identifier\n", 2);
            status = 1;
        }
        else
            remove_env_var(shell, cmd->args[i]);
        i++;
    }
    return (status);
}
