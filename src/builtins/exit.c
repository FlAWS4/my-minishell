/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 04:42:55 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_numeric(const char *str)
{
	int	i;

	i = 0;
	if (!str || !str[0])
		return (0);
	if ((str[0] == '+' || str[0] == '-') && str[1] == '\0')
		return (0);
	if (str[0] == '+' || str[0] == '-')
		i++;
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (0);
		i++;
	}
	return (1);
}

static int	is_valid_long(const char *str)
{
	int				i;
	unsigned long	result;
	unsigned long	limit;

	i = 0;
	result = 0;
	limit = (unsigned long)LONG_MAX;
	if (!str || !str[0])
		return (0);
	if (str[i] == '+' || str[i] == '-')
	{
		if (str[i] == '-')
			limit++;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		if (result > (limit - (str[i] - '0')) / 10)
			return (0);
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (str[i] == '\0');
}

static int	handle_exit_args(t_shell *shell, t_command *cmd, int *arg_exit_code)
{
	if (!cmd->args[1])
		*arg_exit_code = 0;
	else if (!is_numeric(cmd->args[1]) || !is_valid_long(cmd->args[1]))
	{
		error("exit", cmd->args[1], "numeric argument required");
		g_exit_status = 2;
		clean_and_exit_shell(shell, g_exit_status);
	}
	else if (cmd->args[2] != NULL)
	{
		error("exit", NULL, "too many arguments");
		g_exit_status = 1;
		return (1);
	}
	else
		*arg_exit_code = ft_atoi(cmd->args[1]);
	return (0);
}

int	builtin_exit(t_shell *shell, t_command *cmd)
{
	int	arg_exit_code;

	if (!shell || !cmd || !cmd->args)
		return (error("exit", NULL, "internal error"), 1);
	ft_putstr_fd("exit\n", STDOUT_FILENO);
	if (handle_exit_args(shell, cmd, &arg_exit_code))
		return (1);
	g_exit_status = arg_exit_code % 256;
	clean_and_exit_shell(shell, g_exit_status);
	return (0);
}
