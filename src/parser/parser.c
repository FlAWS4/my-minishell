/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:29 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/14 20:38:19 by mshariar         ###   ########.fr       */
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
    return (cmd);
}

/**
 * Allocate initial args array
 */
int	init_args(t_cmd *cmd, char *arg)
{
    cmd->args = malloc(sizeof(char *) * 2);
    if (!cmd->args)
        return (0);
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
        return ;
    i = 0;
    while (cmd->args[i])
    {
        new_args[i] = cmd->args[i];
        i++;
    }
    new_args[i] = arg;
    new_args[i + 1] = NULL;
    free(cmd->args);
    cmd->args = new_args;
}


/**
 * Parse redirection tokens
 
int	parse_redirections(t_token **tokens, t_cmd *cmd)
{
    t_token	*token;
    
    token = *tokens;
    if (token->type == TOKEN_REDIR_IN)
    {
        if (!token->next || token->next->type != TOKEN_WORD)
            return (0);
        cmd->input_file = ft_strdup(token->next->value);
        *tokens = token->next;
    }
    else if (token->type == TOKEN_REDIR_OUT)
    {
        if (!token->next || token->next->type != TOKEN_WORD)
            return (0);
        cmd->output_file = ft_strdup(token->next->value);
        cmd->append_mode = 0;
        *tokens = token->next;
    }
    else if (token->type == TOKEN_REDIR_APPEND)
    {
        if (!token->next || token->next->type != TOKEN_WORD)
            return (0);
        cmd->output_file = ft_strdup(token->next->value);
        cmd->append_mode = 1;
        *tokens = token->next;
    }
    else if (token->type == TOKEN_HEREDOC)
    {
        if (!token->next || token->next->type != TOKEN_WORD)
            return (0);
        cmd->heredoc_delim = ft_strdup(token->next->value);
        *tokens = token->next;
    }
    return (1);
}


 * Parse tokens into command structures

t_cmd	*parse_tokens(t_token *tokens)
{
    t_cmd	*cmd_list;
    t_cmd	*current_cmd;
    t_token	*token;
    
    cmd_list = NULL;
    current_cmd = create_cmd();
    if (!current_cmd)
        return (NULL);
    cmd_list = current_cmd;
    token = tokens;
    while (token)
    {
        if (token->type == TOKEN_WORD)
            add_arg(current_cmd, ft_strdup(token->value));
        else if (token->type == TOKEN_PIPE)
        {
            current_cmd->next = create_cmd();
            if (!current_cmd->next)
                return (cmd_list); // Should handle error better
            current_cmd = current_cmd->next;
        }
        else if (token->type >= TOKEN_REDIR_IN && token->type <= TOKEN_HEREDOC)
        {
            if (!parse_redirections(&token, current_cmd))
                return (cmd_list); // Should handle error better
        }
        token = token->next;
    }
    return (cmd_list);
}\
*/