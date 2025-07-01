/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:51 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/24 01:05:40 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	export_without_value(char *arg, t_shell *shell)
{
	int		var_pos;
	char	*new_var;

	if (!arg || arg[0] == '\0')
	{
		error_quoted ("export", arg, ERROR_IDENTIFIER);
		return (0);
	}
	if (!is_valid_identifier(arg))
	{
		error_quoted("export", arg, ERROR_IDENTIFIER);
		return (1);
	}
	var_pos = find_var_pos(arg, shell);
	if (var_pos == -1)
	{
		new_var = create_managed_string_copy(&shell->memory_manager, arg);
		if (!new_var)
		{
			error(NULL, NULL, ERROR_MALLOC);
			return (1);
		}
		add_to_env(shell, new_var);
	}
	return (0);
}

static	int	export_with_value(char *arg, t_shell *shell)
{
	char	*var_name;
	int		var_pos;
	char	*equal_sign;

	if (!arg || arg[0] == '\0')
		return (1);
	equal_sign = ft_strchr(arg, '=');
	if (!equal_sign)
		return (1);
	var_name = ft_substr(arg, 0, equal_sign - arg);
	if (!var_name)
		return (error(NULL, NULL, ERROR_MALLOC), 1);
	if (!is_valid_identifier(var_name))
		return (error_quoted("export", arg, ERROR_IDENTIFIER),
			free(var_name), 1);
	var_pos = find_var_pos(var_name, shell);
	if (var_pos >= 0)
	{
		shell->env[var_pos] = create_managed_string_copy \
		(&shell->memory_manager, arg);
		if (!shell->env[var_pos])
			return (error(NULL, NULL, ERROR_MALLOC), free(var_name), 1);
	}
	else
		add_to_env(shell, arg);
	free(var_name);
	return (0);
}

static void	print_single_export_line(const char *var)
{
	const char	*equal_sign = ft_strchr(var, '=');

	ft_putstr_fd("export ", STDOUT_FILENO);
	if (equal_sign && *(equal_sign + 1) != '\0')
	{
		write(STDOUT_FILENO, var, equal_sign - var);
		ft_putstr_fd("=\"", STDOUT_FILENO);
		ft_putstr_fd(equal_sign + 1, STDOUT_FILENO);
		ft_putstr_fd("\"\n", STDOUT_FILENO);
	}
	else if (equal_sign && *(equal_sign + 1) == '\0')
	{
		write(STDOUT_FILENO, var, equal_sign - var);
		ft_putstr_fd("=\"\"\n", STDOUT_FILENO);
	}
	else
	{
		ft_putstr_fd(var, STDOUT_FILENO);
		ft_putchar_fd('\n', STDOUT_FILENO);
	}
}

static	void	print_export(t_shell *shell)
{
	char	**env_copy;
	int		var_count;
	int		i;

	i = 0;
	var_count = 0;
	while (shell->env[var_count])
		var_count++;
	env_copy = allocate_managed_memory(&shell->memory_manager, \
		sizeof(char *) * (var_count + 1),
			MEM_ERROR_RECOVERABLE, NULL);
	if (!env_copy)
		return (error(NULL, NULL, ERROR_MALLOC));
	while (i < var_count)
	{
		env_copy[i] = create_managed_string_copy \
		(&shell->memory_manager, shell->env[i]);
		if (!env_copy[i])
			return (error(NULL, NULL, ERROR_MALLOC));
		i++;
	}
	env_copy[i] = NULL;
	sort_env_for_export(env_copy);
	i = 0;
	while (env_copy[i])
		print_single_export_line(env_copy[i++]);
}

int	builtin_export(t_shell *shell, t_command *cmd)
{
	int	i;
	int	status;

	i = 1;
	status = 0;
	g_exit_status = 0;
	if (!shell || !cmd || !cmd->args)
		return (error("export", NULL, "internal error"), 1);
	if (!cmd->args[1])
	{
		print_export(shell);
		return (0);
	}
	while (cmd->args[i])
	{
		if (ft_strchr(cmd->args[i], '='))
			status = export_with_value(cmd->args[i], shell);
		else
			status = export_without_value(cmd->args[i], shell);
		if (status)
			g_exit_status = 1;
		i++;
	}
	return (g_exit_status);
}
