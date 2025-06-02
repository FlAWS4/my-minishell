/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prompt.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 18:49:23 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 04:09:47 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Get shortened path by replacing home directory with ~
 */
static void get_shortened_path(char *result, size_t size)
{
    char cwd[PATH_MAX];
    char *home;
    
    if (!getcwd(cwd, sizeof(cwd)))
    {
        ft_strlcpy(result, "unknown", size);
        return;
    }
    
    home = getenv("HOME");
    if (home && ft_strncmp(cwd, home, ft_strlen(home)) == 0)
    {
        ft_strlcpy(result, "~", size);
        ft_strlcat(result, cwd + ft_strlen(home), size);
    }
    else
    {
        ft_strlcpy(result, cwd, size);
    }
}

/**
 * Create shell prompt string
 */
void create_prompt(char *prompt, int exit_status)
{
    char *username;
    char path[PATH_MAX];
    
    if (!prompt)
        return;
        
    username = getenv("USER");
    if (!username)
        username = "user";
        
    get_shortened_path(path, sizeof(path));
    
    if (exit_status == 0)
        ft_strlcpy(prompt, BOLD_GREEN, 100);
    else
        ft_strlcpy(prompt, BOLD_RED, 100);
        
    ft_strlcat(prompt, username, 100);
    ft_strlcat(prompt, RESET ":" BOLD_BLUE, 100);
    ft_strlcat(prompt, path, 100);
    ft_strlcat(prompt, RESET "$ ", 100);
}

/**
 * Display commands usage
 */
static void display_commands(void)
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
static void display_env_commands(void)
{
    ft_putstr_fd(BOLD_WHITE "env" RESET "          ", 1);
    ft_putstr_fd("Display environment variables\n", 1);
    ft_putstr_fd(BOLD_WHITE "export" RESET " <var>  ", 1);
    ft_putstr_fd("Set environment variable\n", 1);
    ft_putstr_fd(BOLD_WHITE "unset" RESET " <var>   ", 1);
    ft_putstr_fd("Remove environment variable\n", 1);
}

/**
 * Display redirection and pipe usage
 */
static void display_operators(void)
{
    ft_putstr_fd(BOLD_CYAN "\nOperators:\n\n" RESET, 1);
    ft_putstr_fd(BOLD_WHITE "|" RESET "            ", 1);
    ft_putstr_fd("Pipe output to another command\n", 1);
    ft_putstr_fd(BOLD_WHITE ">" RESET " <file>     ", 1);
    ft_putstr_fd("Redirect output to file\n", 1);
    ft_putstr_fd(BOLD_WHITE ">>" RESET " <file>    ", 1);
    ft_putstr_fd("Append output to file\n", 1);
    ft_putstr_fd(BOLD_WHITE "<" RESET " <file>     ", 1);
    ft_putstr_fd("Read input from file\n", 1);
    ft_putstr_fd(BOLD_WHITE "<<" RESET " <delim>   ", 1);
    ft_putstr_fd("Read input until delimiter (heredoc)\n", 1);
}

/**
 * Built-in help command
 */
int builtin_help(t_shell *shell)
{
    (void)shell;
    
    ft_putstr_fd(BOLD_CYAN "\nMinishell Commands:\n\n" RESET, 1);
    display_commands();
    display_env_commands();
    display_operators();
    ft_putstr_fd("\n", 1);
    
    return (0);
}
