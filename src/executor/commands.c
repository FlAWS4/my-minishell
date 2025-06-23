/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:22:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 03:06:51 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*add_texts(char *input, char *str)
{
	char	*s;

	s = ft_strjoin(input, str);
	if (!s)
		return (free(input), NULL);
	return (free(input), s);
}

static int	complete_commands(char **input, char *str, int code)
{
	char	*new_imput;

	new_imput = add_texts(*input, str);
	if (!new_imput)
		return (free(str), free(*input), *input = NULL, 1);
	*input = new_imput;
	return (free(str), code);
}

static char	*read_input(t_shell *data)
{
	char	*str;

	write(STDOUT_FILENO, "> ", 2);
	str = get_next_line(STDIN_FILENO, 0);
	if (g_exit_status == 19)
	{
		data->pipe_interupt = 1;
		g_exit_status = 130;
	}
	return (str);
}

static int	read_line(t_shell *data, char **input)
{
	char	*str;

	if (*input && (*input)[0] == '|')
		return (0);
	str = read_input(data);
	if (data->pipe_interupt)
		return (free(str), free(*input), *input = NULL, 1);
	if (!str)
	{
		write(STDOUT_FILENO, "\nCommand canceled\n", 18);
		return (free(*input), *input = NULL, 1);
	}
	if (str[ft_strlen(str) - 1] == '\n')
		str[ft_strlen(str) - 1] = '\0';
	if (str[0] == '\0' || is_whitespace_bis(str))
		return (free(str), 2);
	if (end_with_pipe(str))
		return (complete_commands(input, str, 3));
	return (complete_commands(input, str, 0));
}

int	get_next_command(t_shell *data, char **input)
{
	struct sigaction	old_int;
	struct sigaction	old_quit;
	int					result;

	data->pipe_interupt = 0;
	init_heredoc_signals(&old_int, &old_quit);
	while (1)
	{
		result = read_line(data, input);
		if (data->pipe_interupt)
		{
			restore_signals_clear_buffer(&old_int, &old_quit);
			return (1);
		}
		if (result == 2 || result == 3)
			continue ;
		if (result != 0)
		{
			restore_signals_clear_buffer(&old_int, &old_quit);
			return (result);
		}
		break ;
	}
	restore_signals_clear_buffer(&old_int, &old_quit);
	return (0);
}
