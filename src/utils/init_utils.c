/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 19:56:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 03:21:41 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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
	if (!cmd)
		return (0);
	return (!ft_strcmp(cmd, "./minishell"));
}

void	update_shell_lvl(t_shell *shell)
{
	char	*current_shlvl;
	int		shlvl;
	char	*new_shlvl;

	current_shlvl = get_env_value(shell, "SHLVL");
	shlvl = 1;
	if (current_shlvl)
	{
		shlvl = ft_atoi(current_shlvl);
		if (shlvl < 0)
			shlvl = 0;
		else
			shlvl++;
	}
	if (shlvl > 999)
	{
		ft_putstr_fd("minishell: warning: shell level (", STDERR_FILENO);
		ft_putnbr_fd(shlvl, STDERR_FILENO);
		ft_putstr_fd(") too high, resetting to 1\n", STDERR_FILENO);
		shlvl = 1;
	}
	new_shlvl = gc_itoa(&shell->gc, shlvl);
	if (!new_shlvl)
		clean_and_exit_shell(shell, EXIT_FAILURE);
	update_env(shell, "SHLVL", new_shlvl);
}

void	clean_and_exit_shell(t_shell *shell, int exit_code)
{
	rl_clear_history();
	if (!shell)
		exit(exit_code);
	if (shell->gc)
		gc_free_all(&shell->gc);
	if (shell->commands)
		free_command(&shell->commands);
	close_fds(shell);
	exit(exit_code);
}
