/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:29 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 03:04:29 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create a new command structure
 */
t_cmd	*create_cmd(void)
{
    t_cmd	*cmd;

    cmd = malloc(sizeof(t_cmd));
    if (!cmd)
        return (NULL);
    cmd->args = NULL;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_mode = 0;
    cmd->heredoc_delim = NULL;
    cmd->next = NULL;
    cmd->redirections = NULL;
    
    // Initialize new fields for better fd tracking
    cmd->input_fd = -1;
    cmd->output_fd = -1;
    cmd->pid = -1;
    
    return (cmd);
}

/**
 * Allocate initial args array
 */
int	init_args(t_cmd *cmd, char *arg)
{
    cmd->args = malloc(sizeof(char *) * 2);
    if (!cmd->args)
    {
        free(arg);
        return (0);
    }
    cmd->args[0] = arg;
    cmd->args[1] = NULL;
    return (1);
}

/**
 * Add argument to command
 */
void	add_arg(t_cmd *cmd, char *arg)
{
    int		i;
    char	**new_args;

    if (!cmd->args)
    {
        if (!init_args(cmd, arg))
            return ;
        return ;
    }
    i = 0;
    while (cmd->args[i])
        i++;
    new_args = malloc(sizeof(char *) * (i + 2));
    if (!new_args)
    {
        free(arg);
        return ;
    }
    i = -1;
    while (cmd->args[++i])
        new_args[i] = cmd->args[i];
    new_args[i] = arg;
    new_args[i + 1] = NULL;
    free(cmd->args);
    cmd->args = new_args;
}

/**
 * Add redirection to command
 */
void	add_redirection(t_cmd *cmd, int type, char *word)
{
    t_redirection	*new;
    t_redirection	*last;
    
    new = malloc(sizeof(t_redirection));
    if (!new)
        return ;
    new->type = type;
    new->word = ft_strdup(word);
    if (!new->word)
    {
        free(new);
        return ;
    }
    new->next = NULL;
    
    // Special handling for heredoc - store in cmd->heredoc_delim too
    // This maintains backward compatibility
    if (type == TOKEN_HEREDOC && !cmd->heredoc_delim)
        cmd->heredoc_delim = ft_strdup(word);
    
    if (!cmd->redirections)
        cmd->redirections = new;
    else
    {
        last = cmd->redirections;
        while (last->next)
            last = last->next;
        last->next = new;
    }
}
