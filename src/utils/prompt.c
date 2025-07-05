/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prompt.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 18:48:15 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 18:48:15 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * generate_dynamic_prompt - 
 * Create a single-line prompt with user info and status
 * @shell: Shell structure containing environment and status information
 *
 * Creates a colorful single-line prompt containing:
 * - Username (from environment)
 * - Current directory (shortened path with ~ for home)
 * - Exit status indicator (colored based on success/failure)
 *
 * Return: Allocated string with formatted prompt (must be freed by caller)
 */
char	*build_prompt_segment(char *user, char *dir_display)
{
	char	*prompt;
	char	*temp;

	if (g_exit_status == 0)
		prompt = ft_strjoin("\001" BOLD_GREEN "\002[", user);
	else
		prompt = ft_strjoin("\001" BOLD_RED "\002[", user);
	temp = prompt;
	prompt = ft_strjoin(prompt, "@minishell ");
	free(temp);
	temp = prompt;
	prompt = ft_strjoin(prompt, "\001" BOLD_BLUE "\002");
	free(temp);
	temp = prompt;
	prompt = ft_strjoin(prompt, dir_display);
	free(temp);
	temp = prompt;
	prompt = ft_strjoin(prompt, "\001" RESET BOLD_GREEN "\002] $ \001" RESET);
	free(temp);
	temp = prompt;
	prompt = ft_strjoin(prompt, "\002");
	free(temp);
	return (prompt);
}

/**
 * Display commands usage with nice formatting
 */
void	display_commands(void)
{
	printf("%s┌─ %sBuilt-in Commands%s ─────────────────────┐%s\n", \
		BOLD_WHITE, BOLD_YELLOW, BOLD_WHITE, RESET);
	printf("│ %s%-12s%s %-28s│\n", BOLD_GREEN, "cd <dir>", RESET, \
		"Change current directory");
	printf("│ %s%-12s%s %-28s│\n", BOLD_GREEN, "pwd", RESET, \
		"Print working directory");
	printf("│ %s%-12s%s %-28s│\n", BOLD_GREEN, "echo <text>", RESET, \
		"Display text");
	printf("│ %s%-12s%s %-28s│\n", BOLD_GREEN, "exit [code]", RESET, \
		"Exit the shell");
	printf("%s└────────────────────────────────────────┘%s\n", \
		BOLD_WHITE, RESET);
}

/**
 * Display environment commands with fancy formatting
 */
void	display_env_commands(void)
{
	printf("%s┌─ %sEnvironment Commands%s ──────────────────┐%s\n", \
		BOLD_WHITE, BOLD_YELLOW, BOLD_WHITE, RESET);
	printf("│ %s%-12s%s %-28s│\n", BOLD_MAGENTA, "env", RESET, \
		"Display environment variables");
	printf("│ %s%-12s%s %-28s│\n", BOLD_MAGENTA, "export <var>", RESET, \
		"Set environment variable");
	printf("│ %s%-12s%s %-28s│\n", BOLD_MAGENTA, "unset <var>", RESET, \
		"Remove environment variable");
	printf("%s└────────────────────────────────────────┘%s\n", \
		BOLD_WHITE, RESET);
}

/**
 * Display redirection and pipe usage with beautiful formatting
 */
void	display_operators(void)
{
	printf("%s┌─ %sOperators%s ────────────────────────────┐%s\n", \
		BOLD_WHITE, BOLD_YELLOW, BOLD_WHITE, RESET);
	printf("│ %s%-12s%s %-28s│\n", BOLD_CYAN, "|", RESET, \
		"Pipe output to another command");
	printf("│ %s%-12s%s %-28s│\n", BOLD_CYAN, "> <file>", RESET, \
		"Redirect output to file");
	printf("│ %s%-12s%s %-28s│\n", BOLD_CYAN, ">> <file>", RESET, \
		"Append output to file");
	printf("│ %s%-12s%s %-28s│\n", BOLD_CYAN, "< <file>", RESET, \
		"Read input from file");
	printf("│ %s%-12s%s %-28s│\n", BOLD_CYAN, "<< <delim>", RESET, \
		"Read input until delimiter");
	printf("%s└────────────────────────────────────────┘%s\n", \
		BOLD_WHITE, RESET);
}

/**
 * Built-in help command with beautiful formatted output
 */
int	builtin_help(t_shell *shell)
{
	(void)shell;
	printf("\n%s✨ %sMINISHELL HELP%s ✨%s\n\n", \
		BOLD_WHITE, BOLD_YELLOW, BOLD_WHITE, RESET);
	display_commands();
	printf("\n");
	display_env_commands();
	printf("\n");
	display_operators();
	printf("\n");
	return (0);
}
