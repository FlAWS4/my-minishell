/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 16:30:18 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 16:30:18 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * remove_useless_token - Removes a token from the linked list
 * @head: Pointer to the head of the token list
 * @token: The token to remove
 */

void	remove_useless_dollars(t_token **head)
{
	t_token	*token;
	t_token	*next;

	token = *head;
	while (token)
	{
		next = token->next;
		if (token->type == WORD && token->value && !token->single_quote
			&& ft_strlen(token->value) < 2 && token->value[0] == '$'
			&& !token->space_after && token->next && (token->next->double_quote
				|| token->next->single_quote) && !token->next->space_before
			&& !token->double_quote)
			remove_useless_token(head, token);
		token = next;
	}
}

/**
 * remove_useless_token - Removes a token from the linked list
 * @head: Pointer to the head of the token list
 * @token: The token to remove
 */

void	clean_empty_tokens(t_token **head)
{
	t_token	*token;
	t_token	*next;

	token = *head;
	while (token)
	{
		next = token->next;
		if (token->type == WORD && token->value && token->value[0] == '\0'
			&& !token->single_quote && !token->double_quote)
			remove_useless_token(head, token);
		token = next;
	}
}

int	is_heredoc(t_redir *redir, t_shell *data)
{
	if (redir->type != HEREDOC)
		return (0);
	redir->heredoc_content = capture_heredoc(redir, data);
	if (data->heredoc_interupt)
		return (1);
	if (!redir->heredoc_content)
	{
		redir->heredoc_content = ft_strdup("");
		if (!redir->heredoc_content)
			return (1);
	}
	return (0);
}

/**
 * get_redir_file - Concatenates the file name from tokens for redirection
 * @tokens: The linked list of tokens
 * @redir: The redirection structure to fill
 *
 * This function traverses the linked list of tokens and concatenates the
 * values of WORD tokens until it encounters a token that is not a WORD.
 * It also checks if the token is quoted outside.
 *
 * Returns the concatenated string or NULL on error.
 */

char	*get_redir_file(t_token *tokens, t_redir *redir)
{
	char	*str;
	char	*tmp;
	t_token	*token;

	str = ft_strdup("");
	if (!str)
		return (NULL);
	token = tokens;
	while (token && token->type == WORD)
	{
		if (token->value)
		{
			tmp = ft_strjoin(str, token->value);
			if (!tmp)
				return (free(str), NULL);
			free(str);
			str = tmp;
		}
		if (token->space_after)
			break ;
		token = token->next;
	}
	if (token->double_quote || token->single_quote || token->quoted_outside)
		redir->quoted_outside = 1;
	return (str);
}

int	is_shell_command(char *cmd)
{
	char	*basename;
	int		i;

	if (!cmd)
		return (0);
	basename = cmd;
	i = 0;
	while (cmd[i])
	{
		if (cmd[i] == '/')
			basename = cmd + i + 1;
		i++;
	}
	return (!ft_strcmp(basename, "minishell") || \
		!ft_strcmp(basename, "bash") || \
		!ft_strcmp(basename, "sh") || \
		!ft_strcmp(basename, "zsh") || \
		!ft_strcmp(basename, "csh") || \
		!ft_strcmp(basename, "tcsh") || \
		!ft_strcmp(basename, "fish"));
}
