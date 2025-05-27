/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prompt.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 18:49:23 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/27 00:02:56 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create shell prompt string
 */
void	create_prompt(char *prompt, int exit_status)
{
    char	*username;
    char	cwd[1024];
    
    username = getenv("USER");
    if (!username)
        username = "user";
        
    if (!getcwd(cwd, sizeof(cwd)))
        ft_strlcpy(cwd, "unknown", sizeof(cwd));
    
    if (exit_status == 0)
        ft_strlcpy(prompt, BOLD_GREEN, 100);
    else
        ft_strlcpy(prompt, BOLD_RED, 100);
        
    ft_strlcat(prompt, username, 100);
    ft_strlcat(prompt, RESET ":" BOLD_BLUE, 100);
    ft_strlcat(prompt, cwd, 100);
    ft_strlcat(prompt, RESET "$ ", 100);
}

/**
 * Display commands usage
 */
static void	display_commands(void)
{
    ft_putstr_fd(BOLD_WHITE "cd" RESET " <dir>     ", 1);
    ft_putstr_fd("Change directory\n", 1);
    ft_putstr_fd(BOLD_WHITE "pwd" RESET "          ", 1);
    ft_putstr_fd("Print working directory\n", 1);
    ft_putstr_fd(BOLD_WHITE "echo" RESET " <text>  ", 1);
    ft_putstr_fd("Display text\n", 1);
    ft_putstr_fd(BOLD_WHITE "exit" RESET " [code]  ", 1);
    ft_putstr_fd("Exit the shell\n", 1);
}

/**
 * Display environment commands
 */
static void	display_env_commands(void)
{
    ft_putstr_fd(BOLD_WHITE "env" RESET "          ", 1);
    ft_putstr_fd("Display environment variables\n", 1);
    ft_putstr_fd(BOLD_WHITE "export" RESET " <var>  ", 1);
    ft_putstr_fd("Set environment variable\n", 1);
    ft_putstr_fd(BOLD_WHITE "unset" RESET " <var>   ", 1);
    ft_putstr_fd("Remove environment variable\n", 1);
}

/**
 * Built-in help command
 */
int	builtin_help(t_shell *shell)
{
    (void)shell;
    
    ft_putstr_fd(BOLD_CYAN "\nMinishell Commands:\n\n" RESET, 1);
    display_commands();
    display_env_commands();
    ft_putstr_fd("\n", 1);
    
    return (0);
}
