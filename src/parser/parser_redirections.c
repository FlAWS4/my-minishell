/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redirections.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:38:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/10 21:44:06 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if token can be used as a redirection target
 */
static int	is_valid_redir_target(t_token *token)
{
    return (token && (token->type == TOKEN_WORD || 
            token->type == TOKEN_SINGLE_QUOTE || 
            token->type == TOKEN_DOUBLE_QUOTE));
}

/**
 * Initialize a new redirection structure
 */
static t_redirection	*init_redirection(int type, char *word, int quoted)
{
    t_redirection	*new;

    new = (t_redirection *)malloc(sizeof(t_redirection));
    if (!new)
        return (NULL);
    new->type = type;
    new->word = ft_strdup(word);
    if (!new->word)
    {
        free(new);
        return (NULL);
    }
    new->input_fd = -1;
    new->output_fd = -1;
    new->quoted = quoted;
    new->temp_file = NULL;
    new->next = NULL;
    return (new);
}

/**
 * Add a redirection to the cmd structure
 * Return 1 for success, 0 for failure
 */
int	add_redirection(t_cmd *cmd, int type, char *word, int quoted)
{
    t_redirection	*new;
    t_redirection	*temp;

    if (!cmd || !word)
        return (0);
    new = init_redirection(type, word, quoted);
    if (!new)
        return (0);
    if (!cmd->redirections)
        cmd->redirections = new;
    else
    {
        temp = cmd->redirections;
        while (temp->next)
            temp = temp->next;
        temp->next = new;
    }
    return (1);
}

/**
 * Handle input redirection
 * Return 1 for success, 0 for failure
 */
int	handle_redir_in(t_token **token, t_cmd *cmd)
{
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
        return (0);
    cmd->input_file = ft_strdup((*token)->next->value);
    if (!cmd->input_file)
        return (0);
    if (!add_redirection(cmd, TOKEN_REDIR_IN, (*token)->next->value, 0))
    {
        free(cmd->input_file);
        cmd->input_file = NULL;
        return (0);
    }
    cmd->input_fd = -1;
    *token = (*token)->next;
    return (1);
}

/**
 * Handle output redirection
 * Return 1 for success, 0 for failure
 */
int	handle_redir_out(t_token **token, t_cmd *cmd, int append)
{
    int	token_type;

    if (!(*token)->next || !is_valid_redir_target((*token)->next))
        return (0);
    cmd->output_file = ft_strdup((*token)->next->value);
    if (!cmd->output_file)
        return (0);
    cmd->append_mode = append;
    if (append)
        token_type = TOKEN_REDIR_APPEND;
    else
        token_type = TOKEN_REDIR_OUT;
    if (!add_redirection(cmd, token_type, (*token)->next->value, 0))
    {
        free(cmd->output_file);
        cmd->output_file = NULL;
        return (0);
    }
    *token = (*token)->next;
    return (1);
}

/**
 * Handle heredoc redirection part 1
 */
static int	handle_heredoc_setup(t_token **token, t_cmd *cmd, int *quoted)
{
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
    {
        display_error(ERR_SYNTAX, "expected delimiter after <<", NULL);
        return (0);
    }
    if ((*token)->next->type == TOKEN_SINGLE_QUOTE || 
        (*token)->next->type == TOKEN_DOUBLE_QUOTE)
        *quoted = 1;
    else
        *quoted = 0;
    cmd->heredoc_delim = ft_strdup((*token)->next->value);
    if (!cmd->heredoc_delim)
    {
        display_error(ERR_MEMORY, NULL, NULL);
        return (0);
    }
    return (1);
}

/**
 * Handle heredoc redirection
 * Return 0 for success, 1 for failure (to match other redirections)
 */
int	handle_heredoc(t_token **token, t_cmd *cmd)
{
    int	quoted;

    quoted = 0;
    if (!handle_heredoc_setup(token, cmd, &quoted))
        return (1);
    cmd->heredoc_file = NULL;
    cmd->input_fd = -1;
    if (!add_redirection(cmd, TOKEN_HEREDOC, (*token)->next->value, quoted))
    {
        free(cmd->heredoc_delim);
        cmd->heredoc_delim = NULL;
        return (1);
    }
    *token = (*token)->next;
    return (0);
}

/**
 * Check if token is a redirection token
 */
int	is_redirection_token(t_token *token)
{
    if (!token)
        return (0);
    return (token->type == TOKEN_REDIR_IN ||
            token->type == TOKEN_REDIR_OUT ||
            token->type == TOKEN_REDIR_APPEND ||
            token->type == TOKEN_HEREDOC);
}

/**
 * Parse redirection tokens
 * Return 0 for success, 1 for failure
 */
int	parse_redirections(t_token **token, t_cmd *cmd)
{
    t_token_type	type;
    int				result;

    if (!token || !*token || !cmd)
        return (1);
    type = (*token)->type;
    if (type == TOKEN_HEREDOC)
        result = handle_heredoc(token, cmd);
    else if (type == TOKEN_REDIR_IN)
        result = !handle_redir_in(token, cmd);
    else if (type == TOKEN_REDIR_OUT)
        result = !handle_redir_out(token, cmd, 0);
    else if (type == TOKEN_REDIR_APPEND)
        result = !handle_redir_out(token, cmd, 1);
    else
        result = 1;
    return (result);
}
