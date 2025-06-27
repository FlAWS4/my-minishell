/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirection_list.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 15:42:56 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 15:42:56 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	add_ambiguous_redirect(t_redir **redirs, t_token *tokens)
{
	t_redir	*redir;

	if (tokens->type == HEREDOC)
		return (0);
	redir = malloc(sizeof(t_redir));
	if (!redir)
		return (1);
	ft_memset(redir, 0, sizeof(t_redir));
	redir->type = tokens->type;
	ft_putstr_fd("minishell: ambiguous redirect\n", 2);
	g_exit_status = 1;
	redir->ar = 1;
	redir->next = NULL;
	add_redirs(redirs, redir);
	return (0);
}

void	check_ambiguous_redirect(t_redir *cmd, t_token *tokens)
{
	t_token	*token;

	token = tokens;
	if (token->type == HEREDOC)
		return ;
	if (token->type == REDIR_IN || token->type == REDIR_OUT
		|| token->type == APPEND)
	{
		if (token->next && token->next->type != WORD)
		{
			ft_putstr_fd("minishell: ambiguous redirect\n", 2);
			g_exit_status = 1;
			cmd->ar = 1;
			return ;
		}
	}
	if (token->next && token->next->type == WORD && token->next->ar)
	{
		ft_putstr_fd("minishell: ambiguous redirect\n", 2);
		g_exit_status = 1;
		cmd->ar = 1;
		return ;
	}
}