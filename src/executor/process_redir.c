/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   process_redir.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 23:45:45 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 04:45:28 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check line against delimiter for heredoc (discard version)
 */
static int	check_heredoc_discard(char *line, char *delimiter)
{
    int	len;

    if (!line)
        return (0);
    len = ft_strlen(line);
    if (len > 0 && line[len - 1] == '\n')
        line[len - 1] = '\0';
    if (ft_strcmp(line, delimiter) == 0)
    {
        free(line);
        return (1);
    }
    // Just discard the line, don't write it anywhere
    free(line);
    return (0);
}

/**
 * Collect heredoc content but discard it (for non-last heredocs)
 */
int	collect_and_discard_heredoc(char *delimiter)
{
    char	*line;
    pid_t   pid;
    int     status;

    // Use fork to handle signal interruption properly
    pid = fork();
    if (pid == -1)
        return (0);
    
    if (pid == 0)
    {
        // Child process
        setup_signals_heredoc();
        reset_gnl_buffer();
        
        while (1)
        {
            // CRITICAL CHANGE: Use consistent prompt
            ft_putstr_fd("heredoc> ", STDOUT_FILENO);
            line = get_next_line(STDIN_FILENO);
            
            if (!line)
                exit(0);
                
            if (check_heredoc_discard(line, delimiter))
                exit(0);
        }
        // Never reached
        exit(0);
    }
    
    // Parent process
    signal(SIGINT, SIG_IGN);
    waitpid(pid, &status, 0);
    setup_signals();
    
    // Check if child was interrupted
    if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
    {
        g_signal = SIGINT;
        return (0);
    }
    
    return (1);
}

/**
 * Process all redirections in a command's redirection list
 */
int process_redirections(t_cmd *cmd)
{
    t_redirection *current;
    
    if (!cmd || !cmd->redirections)
        return (0);
        
    current = cmd->redirections;
    while (current)
    {
        if (current->type == TOKEN_REDIR_IN)
        {
            if (process_input_redir(current) != 0)
                return (1);
        }
        else if (current->type == TOKEN_REDIR_OUT || current->type == TOKEN_REDIR_APPEND)
        {
            if (process_output_redir(current) != 0)
                return (1);
        }
        else if (current->type == TOKEN_HEREDOC)
        {
            if (process_heredoc_redir(current) != 0)
                return (1);
        }
        
        current = current->next;
    }
    
    return (0);
}
