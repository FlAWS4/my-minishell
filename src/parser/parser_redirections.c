/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redirections.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:38:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/24 01:31:10 by mshariar         ###   ########.fr       */
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
static int	concatane_string(char **str, char *line)
{
	char	*tmp;

	if (!*str)
	{
		*str = ft_strdup("");
		if (!*str)
			return (1);
	}
	tmp = ft_strjoin(*str, line);
	free(line);
	if (!tmp)
		return (free(*str), 1);
	free(*str);
	*str = tmp;
	tmp = ft_strjoin(*str, "\n");
	if (!tmp)
		return (free(*str), 1);
	free(*str);
	*str = tmp;
	return (0);
}

static char	*have_to_expand(t_shell *data, char *line, t_redir *redir)
{
	char	*str;

	if (!line)
		return (NULL);
	if (redir->quoted || redir->quoted_outside)
		return (line);
	str = expand_variables(data, line, redir);
	free(line);
	if (!str)
		return (NULL);
	return (str);
}

static char	*get_line(t_shell *data)
{
	char	*str;

	write(STDOUT_FILENO, "> ", 2);
	str = get_next_line(STDIN_FILENO, 0);
	if (g_exit_status == 19)
	{
		data->heredoc_interupt = 1;
		g_exit_status = 130;
	}
	return (str);
}

static char	*read_and_store_heredoc(t_redir *redir, t_shell *data,
	t_char *strings)
{
	while (1)
	{
		strings->line = get_line(data);
		if (data->heredoc_interupt)
			return (free(strings->line), free(strings->str), NULL);
		if (!strings->line)
		{
			display_heredoc_eof_warning(redir->file_or_del);
			break ;
		}
		if (strings->line[ft_strlen(strings->line) - 1] == '\n')
			strings->line[ft_strlen(strings->line) - 1] = '\0';
		if (!ft_strcmp(strings->line, redir->file_or_del))
			return (free(strings->line), strings->str);
		strings->new_line = have_to_expand(data, strings->line, redir);
		if (!strings->new_line)
			return (free(strings->str), NULL);
		if (concatane_string(&strings->str, strings->new_line))
			return (free(strings->str), NULL);
	}
	return (free(strings->line), strings->str);
}

char	*capture_heredoc(t_redir *redir, t_shell *data)
{
	char				*str;
	struct sigaction	old_int;
	struct sigaction	old_quit;
	t_char				strings;

	strings.str = NULL;
	data->heredoc_interupt = 0;
	setup_heredoc_signal_handlers(&old_int, &old_quit);
	disable_control_char_echo();
	str = read_and_store_heredoc(redir, data, &strings);
	sigaction(SIGINT, &old_int, NULL);
	sigaction(SIGQUIT, &old_quit, NULL);
	get_next_line(STDIN_FILENO, 1);
	enable_control_char_echo();
	return (str);
}
