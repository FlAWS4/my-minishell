/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_free.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 15:46:16 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 15:46:16 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Display heredoc EOF warning
 */

void	free_redirs(t_redir **redirs)
{
	t_redir	*current;
	t_redir	*next;

	current = *redirs;
	while (current)
	{
		next = current->next;
		if (current->file_or_del)
			free(current->file_or_del);
		if (current->heredoc_content)
			free(current->heredoc_content);
		free(current);
		current = next;
	}
	*redirs = NULL;
}

void	free_tokens_list(t_token **head)
{
	t_token	*current;
	t_token	*next;

	current = *head;
	while (current)
	{
		next = current->next;
		if (current->value)
			free(current->value);
		free(current);
		current = next;
	}
	*head = NULL;
}

void	free_array(void *ptr)
{
	int		i;
	char	**tab;

	if (!ptr)
		return ;
	tab = (char **)ptr;
	i = 0;
	while (tab[i])
	{
		free(tab[i]);
		i++;
	}
	free(tab);
}

void	free_command(t_command **cmds)
{
	t_command	*cmd;
	t_command	*next;

	if (!cmds || !*cmds)
		return ;
	cmd = *cmds;
	while (cmd)
	{
		next = cmd->next;
		if (cmd->args)
			free_array(cmd->args);
		if (cmd->redirs)
			free_redirs(&cmd->redirs);
		free(cmd);
		cmd = next;
	}
	*cmds = NULL;
}