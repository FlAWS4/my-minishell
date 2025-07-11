/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 18:47:33 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 18:47:33 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * add_command - Adds a command to the linked list of commands
 * @cmds: Pointer to the head of the linked list of commands
 * @cmd: The command to be added
 *
 * If the list is empty, it initializes the head with the new command.
 * Otherwise, it traverses to the end of the list and adds the new command.
 * Sets default file descriptors for input and output.
 */

static void	add_command(t_command **cmds, t_command *cmd)
{
	t_command	*current;

	if (!*cmds)
	{
		*cmds = cmd;
		cmd->fd_in = STDIN_FILENO;
		cmd->fd_out = STDOUT_FILENO;
		return ;
	}
	current = *cmds;
	while (current->next)
		current = current->next;
	current->next = cmd;
	cmd->previous = current;
	cmd->fd_in = STDIN_FILENO;
	cmd->fd_out = STDOUT_FILENO;
	cmd->next = NULL;
}

/**
 * count_words - Counts the number of WORD tokens in the token list
 * @data: Pointer to the shell data structure
 *
 * This function iterates through the tokens and counts how many WORD tokens
 * are present, excluding operators and special tokens.
 * Returns the count of WORD tokens.
 */

static int	handle_args(t_token *tokens, char **args)
{
	if (!is_cmd(tokens))
		return (0);
	else if (fill_words(tokens, args))
		return (1);
	return (0);
}

static int	fill_command_args(t_shell *data, char **args)
{
	t_token	*token;
	int		i;
	char	*str;

	str = NULL;
	token = data->tokens;
	i = 0;
	while (token && token->type != PIPE && token->type != T_EOF)
	{
		if (token->type == WORD
			&& (!token->previous || !is_token_operator(token->previous->type)))
		{
			if (handle_args(token, &str))
				return (1);
			if (str)
				args[i++] = str;
		}
		token = token->next;
	}
	args[i] = NULL;
	return (0);
}

static int	create_command(t_shell *data, t_command **cmds)
{
	t_command	*cmd;
	int			args_nb;

	cmd = malloc(sizeof(t_command));
	if (!cmd)
		return (1);
	ft_memset(cmd, 0, sizeof(t_command));
	args_nb = count_words(data);
	if (args_nb > 0)
	{
		cmd->args = malloc(sizeof(char *) * (args_nb + 1));
		if (!cmd->args)
			return (free_command(&cmd), 1);
		ft_memset(cmd->args, 0, (sizeof(char *) * (args_nb + 1)));
		if (fill_command_args(data, cmd->args))
			return (free_command(&cmd), 1);
	}
	cmd->redirs = init_redir(data);
	if (!cmd->redirs && data->heredoc_interupt)
		return (free_command(&cmd), 1);
	add_command(cmds, cmd);
	return (0);
}

int	split_cmd_with_pipe(t_shell *data)
{
	int			pipe_nb;
	t_command	*cmds;
	t_token		*token;
	t_token		*head;

	if (process_and_join_tokens(data))
		return (1);
	head = data->tokens;
	token = data->tokens;
	cmds = NULL;
	pipe_nb = count_pipe(data);
	while (pipe_nb-- && (token && token->type != T_EOF))
	{
		data->tokens = token;
		if (create_command(data, &cmds))
			return (free_command(&cmds), free_tokens_list(&data->tokens), 1);
		while (token && token->type != PIPE && token->type != T_EOF)
			token = token->next;
		if (token && token->type == PIPE)
			token = token->next;
	}
	data->commands = cmds;
	free_tokens_list(&head);
	return (0);
}
