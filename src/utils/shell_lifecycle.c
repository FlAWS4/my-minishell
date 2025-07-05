/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shell_lifecycle.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 16:30:57 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 16:30:57 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * print_shlvl_warning - Prints warning when shell level is too high
 * @shlvl: The current shell level value
 */
static void	print_shlvl_warning(int shlvl)
{
	ft_putstr_fd(BOLD_YELLOW "minishell: warning: shell level (", \
		STDERR_FILENO);
	ft_putnbr_fd(shlvl, STDERR_FILENO);
	ft_putstr_fd(") too high, resetting to 1\n" RESET, STDERR_FILENO);
}

/**
 * update_shell_lvl - Updates the SHLVL environment variable
 * Increments the shell level, resets to 1 if it exceeds 999,
 * and prints a warning message if it does.
 */

void	update_shell_lvl(t_shell *shell)
{
	char	*current_shlvl;
	int		shlvl;
	char	*new_shlvl;

	current_shlvl = get_env_value(shell, "SHLVL");
	if (current_shlvl)
	{
		shlvl = ft_atoi(current_shlvl);
		if (shlvl < 0)
			shlvl = 0;
		shlvl++;
	}
	else
		shlvl = 1;
	if (shlvl > 999)
	{
		print_shlvl_warning(shlvl);
		shlvl = 1;
	}
	new_shlvl = gc_itoa(&shell->memory_manager, shlvl);
	if (!new_shlvl)
		clean_and_exit_shell(shell, EXIT_FAILURE);
	update_env(shell, "SHLVL", new_shlvl);
}

void	clean_and_exit_shell(t_shell *shell, int exit_code)
{
	rl_clear_history();
	if (!shell)
		exit(exit_code);
	if (shell->default_path)
	{
		free(shell->default_path);
		shell->default_path = NULL;
	}
	if (shell->memory_manager)
		release_all_memory(&shell->memory_manager);
	if (shell->commands)
		free_command(&shell->commands);
	cleanup_shell_file_descriptors(shell);
	exit(exit_code);
}

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	i;

	i = 0;
	if (n == 0)
		return (0);
	while (i < n - 1 && s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

void	*ft_memcpy(void *dest, const void *src, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
		i++;
	}
	return (dest);
}
