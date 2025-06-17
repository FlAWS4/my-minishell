/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:34:36 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 02:04:53 by mshariar         ###   ########.fr       */
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
        return ;
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
 * Handle invalid identifier for unset command
 */
static int	handle_invalid_id(char *arg)
{
    display_error(ERROR_UNSET, arg, "not a valid identifier");
    return (1);
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
    if (!cmd->args[i])
        return (0);
    while (cmd->args[i])
    {
        if (!is_valid_identifier(cmd->args[i]))
            status = handle_invalid_id(cmd->args[i]);
        else
            remove_env_var(shell, cmd->args[i]);
        i++;
    }
    return (status);
}
