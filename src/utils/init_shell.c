/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_shell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 22:42:29 by mshariar          #+#    #+#             */
/*   Updated: 2025/07/03 21:42:45 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * backup_standard_fds - Save copies of standard file descriptors
 * @shell: Shell structure to store file descriptor copies
 *
 * Duplicates STDIN and STDOUT for restoration after redirections
 * Exits with error if duplication fails
 */
void	init_shell_fds(t_shell *shell)
{
	shell->saved_stdin = dup(STDIN_FILENO);
	shell->saved_stdout = dup(STDOUT_FILENO);
	if (shell->saved_stdin == -1 || shell->saved_stdout == -1)
	{
		error("dup", NULL, strerror(errno));
		clean_and_exit_shell(shell, 1);
	}
}

/**
 * format_shell_prompt - 
 * Generate a formatted shell prompt with user and current directory
 * @shell: Shell structure containing environment variables
 *
 * Constructs a prompt string that includes:
 * - Username from environment variable USER
 * - Current working directory (PWD), shortened with ~ for home directory
 * 
 * Returns: Allocated string with formatted prompt (must be freed by caller)
 */
char	*get_current_directory(t_shell *shell)
{
	char	*cwd;

	cwd = get_env_value(shell, "PWD");
	if (cwd)
		return (ft_strdup(cwd));
	cwd = getcwd(NULL, 0);
	if (!cwd)
		return (ft_strdup("unknown"));
	return (cwd);
}

char	*format_shell_prompt(t_shell *shell)
{
	char	*cwd;
	char	*user;
	char	*prompt;
	char	*home_dir;
	char	*dir_display;

	user = get_env_value(shell, "USER");
	if (!user)
		user = "user";
	cwd = get_current_directory(shell);
	home_dir = get_env_value(shell, "HOME");
	if (home_dir && ft_strncmp(cwd, home_dir, ft_strlen(home_dir)) == 0)
	{
		dir_display = ft_strjoin("~", cwd + ft_strlen(home_dir));
		free(cwd);
	}
	else
		dir_display = cwd;
	prompt = build_prompt_segment(user, dir_display);
	free(dir_display);
	return (prompt);
}
