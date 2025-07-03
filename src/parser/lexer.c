/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 18:47:24 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 18:47:24 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	count_pipe(t_shell *data)
{
	t_token	*token;
	int		i;

	i = 1;
	token = data->tokens;
	while (token)
	{
		if (token->type == PIPE && token->next && token->next->type != PIPE
			&& token->next->type != T_EOF)
			i++;
		token = token->next;
	}
	return (i);
}

int	count_words(t_shell *data)
{
	t_token	*token;
	int		word_nb;

	token = data->tokens;
	word_nb = 0;
	while (token && token->type != PIPE && token->type != T_EOF)
	{
		if (token->type == WORD
			&& (!token->previous || !is_token_operator(token->previous->type)))
			word_nb++;
		token = token->next;
	}
	return (word_nb);
}

/**
 * add_redirs - Adds a redirection to the end of the linked list of redirections
 * @head: Pointer to the head of the linked list of redirections
 * @redir: The redirection to add
 *
 * If the head is NULL, it initializes it with the new redirection.
 * Otherwise, it traverses to the end of the list and adds the new redirection.
 */

void	add_redirs(t_redir **head, t_redir *redir)
{
	t_redir	*current;

	if (!*head)
	{
		*head = redir;
		return ;
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = redir;
	redir->next = NULL;
}

/**
 * create_redir - Creates a new redirection from the given token
 * @redirs: Pointer to the head of the linked list of redirections
 * @tokens: The token from which to create the redirection
 * @data: The shell data structure containing tokens and other information
 *
 * Allocates memory for a new redirection, sets its type and file or delimiter,
 * checks for heredoc, and adds it to the linked list of redirections.
 * Returns 0 on success, 1 on failure.
 */

static int	create_redir(t_redir **redirs, t_token *tokens, t_shell *data)
{
	t_redir	*redir;

	redir = malloc(sizeof(t_redir));
	if (!redir)
		return (1);
	ft_memset(redir, 0, sizeof(t_redir));
	if (tokens->next && tokens->next->type != T_EOF)
	{
		if (tokens->next && tokens->next->single_quote)
			redir->quoted = 1;
		else if (tokens->next && tokens->next->double_quote)
			redir->quoted = 2;
		else if (tokens->next && tokens->next->quoted_outside)
			redir->quoted_outside = 1;
		redir->type = tokens->type;
		redir->file_or_del = get_redir_file(tokens->next, redir);
		if (!redir->file_or_del)
			return (1);
		if (is_heredoc(redir, data))
			return (free_redirs(&redir), 1);
		detect_ambiguous_redirect(redir, tokens);
		redir->next = NULL;
	}
	add_redirs(redirs, redir);
	return (0);
}

/**
 * init_redir - Initializes the redirection list from the shell data
 * @data: The shell data structure containing tokens and other information
 *
 * Iterates through the tokens, creating redirection nodes for valid
 * redirection operators followed by a word token. If an operator is found
 * without a valid word, it creates an ambiguous redirect error.
 * Returns a pointer to the head of the redirection list or NULL on failure.
 */

t_redir	*init_redir(t_shell *data)
{
	t_redir	*redir;
	t_token	*token;

	token = data->tokens;
	redir = NULL;
	while (token)
	{
		if (token->type == PIPE || token->type == T_EOF)
			break ;
		if (is_token_operator(token->type) && token->next
			&& token->next->type == WORD)
		{
			if (create_redir(&redir, token, data))
				return (free_redirs(&redir), NULL);
		}
		else if (is_token_operator(token->type) && token->next
			&& token->next->type != WORD)
		{
			if (create_ambiguous_redirect_error(&redir, token))
				return (free_redirs(&redir), NULL);
		}
		token = token->next;
	}
	return (redir);
}
