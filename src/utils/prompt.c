/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prompt.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 18:49:23 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/10 22:00:07 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Get shortened path by replacing home directory with ~
 */
static void	get_shortened_path(char *result, size_t size)
{
    char	cwd[PATH_MAX];
    char	*home;

    if (!getcwd(cwd, sizeof(cwd)))
    {
        ft_strlcpy(result, "unknown", size);
        return ;
    }
    home = getenv("HOME");
    if (home && ft_strncmp(cwd, home, ft_strlen(home)) == 0)
    {
        ft_strlcpy(result, "~", size);
        ft_strlcat(result, cwd + ft_strlen(home), size);
    }
    else
        ft_strlcpy(result, cwd, size);
}

/**
 * Create beautiful shell prompt string with colors
 */
void	create_prompt(char *prompt, int exit_status)
{
    char	*username;
    char	path[PATH_MAX];

    if (!prompt)
        return ;
    username = getenv("USER");
    if (!username)
        username = "user";
    get_shortened_path(path, sizeof(path));
    if (exit_status == 0)
        sprintf(prompt, "%s[%s%s%s]%s:%s[%s%s%s]%s$ ", 
            BOLD_WHITE, BOLD_GREEN, username, BOLD_WHITE, RESET, 
            BOLD_WHITE, BOLD_BLUE, path, BOLD_WHITE, RESET);
    else
        sprintf(prompt, "%s[%s%s%s]%s:%s[%s%s%s]%s$ ", 
            BOLD_WHITE, BOLD_RED, username, BOLD_WHITE, RESET, 
            BOLD_WHITE, BOLD_BLUE, path, BOLD_WHITE, RESET);
}

/**
 * Display commands usage with nice formatting
 */
static void	display_commands(void)
{
    printf("%s┌─ %sBuilt-in Commands%s ─────────────────────┐%s\n", 
        BOLD_WHITE, BOLD_YELLOW, BOLD_WHITE, RESET);
    printf("│ %s%-12s%s %-28s│\n", BOLD_GREEN, "cd <dir>", RESET, 
        "Change current directory");
    printf("│ %s%-12s%s %-28s│\n", BOLD_GREEN, "pwd", RESET, 
        "Print working directory");
    printf("│ %s%-12s%s %-28s│\n", BOLD_GREEN, "echo <text>", RESET, 
        "Display text");
    printf("│ %s%-12s%s %-28s│\n", BOLD_GREEN, "exit [code]", RESET, 
        "Exit the shell");
    printf("%s└────────────────────────────────────────┘%s\n", 
        BOLD_WHITE, RESET);
}

/**
 * Display environment commands with fancy formatting
 */
static void	display_env_commands(void)
{
    printf("%s┌─ %sEnvironment Commands%s ──────────────────┐%s\n", 
        BOLD_WHITE, BOLD_YELLOW, BOLD_WHITE, RESET);
    printf("│ %s%-12s%s %-28s│\n", BOLD_MAGENTA, "env", RESET, 
        "Display environment variables");
    printf("│ %s%-12s%s %-28s│\n", BOLD_MAGENTA, "export <var>", RESET, 
        "Set environment variable");
    printf("│ %s%-12s%s %-28s│\n", BOLD_MAGENTA, "unset <var>", RESET, 
        "Remove environment variable");
    printf("%s└────────────────────────────────────────┘%s\n", 
        BOLD_WHITE, RESET);
}

/**
 * Display redirection and pipe usage with beautiful formatting
 */
static void	display_operators(void)
{
    printf("%s┌─ %sOperators%s ────────────────────────────┐%s\n", 
        BOLD_WHITE, BOLD_YELLOW, BOLD_WHITE, RESET);
    printf("│ %s%-12s%s %-28s│\n", BOLD_CYAN, "|", RESET, 
        "Pipe output to another command");
    printf("│ %s%-12s%s %-28s│\n", BOLD_CYAN, "> <file>", RESET, 
        "Redirect output to file");
    printf("│ %s%-12s%s %-28s│\n", BOLD_CYAN, ">> <file>", RESET, 
        "Append output to file");
    printf("│ %s%-12s%s %-28s│\n", BOLD_CYAN, "< <file>", RESET, 
        "Read input from file");
    printf("│ %s%-12s%s %-28s│\n", BOLD_CYAN, "<< <delim>", RESET, 
        "Read input until delimiter");
    printf("%s└────────────────────────────────────────┘%s\n", 
        BOLD_WHITE, RESET);
}

/**
 * Built-in help command with beautiful formatted output
 */
int	builtin_help(t_shell *shell)
{
    (void)shell;

    printf("\n%s✨ %sMINISHELL HELP%s ✨%s\n\n", 
        BOLD_WHITE, BOLD_YELLOW, BOLD_WHITE, RESET);
    display_commands();
    printf("\n");
    display_env_commands();
    printf("\n");
    display_operators();
    printf("\n");
    return (0);
}
