/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   history.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 23:44:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/16 03:04:12 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Get path to history file
 * Returns allocated string with path or NULL on error
 */
static char *get_history_path(void)
{
    char *home_dir;
    char *history_path;

    home_dir = getenv("HOME");
    if (!home_dir)
        return (NULL);
    
    history_path = ft_strjoin(home_dir, "/.minishell_history");
    return (history_path);
}

/**
 * Initialize history from file
 * Loads command history from ~/.minishell_history
 */
void	init_history(void)
{
    char	*history_path;
    int     result;

    history_path = get_history_path();
    if (!history_path)
        return;
        
    result = read_history(history_path);
    if (result != 0 && access(history_path, F_OK) == 0)
    {
        // History file exists but couldn't be read
        ft_putstr_fd("Warning: Could not read history file\n", STDERR_FILENO);
    }
    
    free(history_path);
}

/**
 * Save history to file
 * Writes command history to ~/.minishell_history
 */
void	save_history(void)
{
    char	*history_path;
    int     result;
    int     history_fd;

    history_path = get_history_path();
    if (!history_path)
        return;
        
    // Create directory if it doesn't exist
    if (access(history_path, F_OK) != 0)
    {
        // File doesn't exist, create it with appropriate permissions
        history_fd = open(history_path, O_CREAT | O_WRONLY, 0600);
        if (history_fd != -1)
            close(history_fd);
    }
    
    result = write_history(history_path);
    if (result != 0)
    {
        ft_putstr_fd("Warning: Could not write history file\n", STDERR_FILENO);
    }
    
    free(history_path);
}

/**
 * Add command to history if not empty and not duplicate
 * Skips commands that are just whitespace
 */
void	add_to_history(char *cmd)
{
    HIST_ENTRY *last_entry;
    int i;
    
    if (!cmd || !*cmd)
        return;
        
    // Skip if command is just whitespace
    i = 0;
    while (cmd[i] && is_whitespace(cmd[i]))
        i++;
        
    if (!cmd[i])
        return;
        
    // Skip if command is a duplicate of previous command
    last_entry = history_get(history_length);
    if (last_entry && ft_strcmp(last_entry->line, cmd) == 0)
        return;
        
    add_history(cmd);
}

/**
 * Built-in history command
 * Displays command history
 */
int builtin_history(t_shell *shell)
{
    HIST_ENTRY **hist_entries;
    int i;
    
    (void)shell;  // Unused parameter
    
    hist_entries = history_list();  // This is the readline function
    if (!hist_entries)
        return (1);  // Return error code if history list can't be obtained
        
    for (i = 0; hist_entries[i]; i++)
    {
        ft_putnbr_fd(i + 1, STDOUT_FILENO);
        ft_putstr_fd("  ", STDOUT_FILENO);
        ft_putendl_fd(hist_entries[i]->line, STDOUT_FILENO);
    }
    
    return (0);
}

