/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/17 20:50:26 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Set up input redirection
 */
static int	setup_input(t_cmd *cmd)
{
    int	fd;

    if (!cmd->input_file)
        return (0);
    fd = open(cmd->input_file, O_RDONLY);
    if (fd == -1)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(cmd->input_file, 2);
        ft_putstr_fd(": ", 2);
        ft_putstr_fd(strerror(errno), 2);
        ft_putstr_fd("\n", 2);
        return (1);
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
    return (0);
}

/**
 * Set up output redirection
 */
static int	setup_output(t_cmd *cmd)
{
    int	fd;
    int	flags;

    if (!cmd->output_file)
        return (0);
    flags = O_WRONLY | O_CREAT;
    if (cmd->append_mode)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;
    fd = open(cmd->output_file, flags, 0644);
    if (fd == -1)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(cmd->output_file, 2);
        ft_putstr_fd(": ", 2);
        ft_putstr_fd(strerror(errno), 2);
        ft_putstr_fd("\n", 2);
        return (1);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    return (0);
}

/**
 * Set up all redirections for a command
 */
int	setup_redirections(t_cmd *cmd)
{
    if (setup_input(cmd) != 0)
        return (1);
    if (setup_output(cmd) != 0)
        return (1);
    return (0);
}