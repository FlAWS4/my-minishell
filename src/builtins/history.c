/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   history.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 23:44:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/26 23:45:00 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Initialize history from file
 */
void	init_history(void)
{
    char	*home_dir;
    char	history_path[1024];

    home_dir = getenv("HOME");
    if (!home_dir)
        return ;
    ft_strlcpy(history_path, home_dir, sizeof(history_path));
    ft_strlcat(history_path, "/.minishell_history", sizeof(history_path));
    read_history(history_path);
}

/**
 * Save history to file
 */
void	save_history(void)
{
    char	*home_dir;
    char	history_path[1024];

    home_dir = getenv("HOME");
    if (!home_dir)
        return ;
    ft_strlcpy(history_path, home_dir, sizeof(history_path));
    ft_strlcat(history_path, "/.minishell_history", sizeof(history_path));
    write_history(history_path);
}

/**
 * Add command to history if not empty
 */
void	add_to_history(char *cmd)
{
    if (cmd && *cmd)
        add_history(cmd);
}