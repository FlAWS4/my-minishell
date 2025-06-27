/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 04:47:25 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_valid_n_option(const char *arg)
{
	int	i;

	i = 1;
	if (!arg || arg[0] != '-' || arg[1] != 'n')
		return (0);
	while (arg[i] == 'n')
		i++;
	if (arg[i] == '\0')
		return (1);
	else
		return (0);
}

static int	print_echo_args(char **args, int i)
{
	int	is_first_arg;

	is_first_arg = 1;
	while (args[i])
	{
		if (!is_first_arg)
			ft_putchar_fd(' ', STDOUT_FILENO);
		ft_putstr_fd(args[i], STDOUT_FILENO);
		is_first_arg = 0;
		i++;
	}
	return (0);
}

int		builtin_echo(t_command *cmd)
{
	int	i;
	int	print_newline;

	i = 1;
	print_newline = 1;
	if (!cmd || !cmd->args || !cmd->args[0])
		return (0);
	while (cmd->args[i] && is_valid_n_option(cmd->args[i]))
	{
		print_newline = 0;
		i++;
	}
	print_echo_args(cmd->args, i);
	if (print_newline == 1)
	{
		if (ft_putchar_fd('\n', STDOUT_FILENO) != 0)
			return (1);
	}
	return (0);
}
