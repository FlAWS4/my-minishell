/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 06:17:10 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Collect input for heredoc until delimiter is encountered
 */
int collect_heredoc_input(char *delimiter, int fd)
{
    char *line;
    pid_t pid;
    int status;
    
    // Fork to handle signals properly in heredoc
    pid = fork();
    if (pid == -1)
        return (0);
    
    if (pid == 0) 
    {
        // Child process
        setup_signals_heredoc();
        
        // Prompt for input
        ft_putstr_fd("heredoc> ", STDOUT_FILENO);
        
        // Read lines until delimiter is found
        while (1)
        {
            line = get_next_line(STDIN_FILENO);
            
            // Check for EOF or error
            if (!line)
                exit(1);
            
            // Remove trailing newline if present
            int len = ft_strlen(line);
            if (len > 0 && line[len - 1] == '\n')
                line[len - 1] = '\0';
            
            // Check if line equals delimiter
            if (ft_strcmp(line, delimiter) == 0)
            {
                free(line);
                exit(0);
            }
            
            // Write line to heredoc file with newline
            ft_putstr_fd(line, fd);
            ft_putstr_fd("\n", fd);
            
            free(line);
            ft_putstr_fd("heredoc> ", STDOUT_FILENO);
        }
    }
    
    // Parent process
    signal(SIGINT, SIG_IGN);  // Ignore signals while waiting
    waitpid(pid, &status, 0);
    setup_signals();  // Restore signal handlers
    
    // Check if child was terminated by a signal (like SIGINT)
    if (WIFSIGNALED(status))
    {
        g_signal = SIGINT;
        return (0);
    }
    
    // Check if child exited with an error
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        return (0);
    
    return (1);
}

int process_heredoc(t_cmd *cmd)
{
    int fd;
    char *temp_file;
    
    if (!cmd->heredoc_delim)
        return (0);
        
    // Create a temporary file for the heredoc
    temp_file = ft_strdup("/tmp/minishell_heredoc_XXXXXX");
    if (!temp_file)
        return (0);
        
    fd = mkstemp(temp_file);
    if (fd == -1)
    {
        free(temp_file);
        return (0);
    }
    
    // Store the file path for later cleanup
    cmd->heredoc_file = temp_file;
    
    // Collect the heredoc input
    if (!collect_heredoc_input(cmd->heredoc_delim, fd))
    {
        close(fd);
        unlink(temp_file);
        free(temp_file);
        cmd->heredoc_file = NULL;
        return (0);
    }
    
    close(fd);
    
    // Open the file for reading and set input_fd
    fd = open(temp_file, O_RDONLY);
    if (fd == -1)
    {
        unlink(temp_file);
        free(temp_file);
        cmd->heredoc_file = NULL;
        return (0);
    }
    
    // Store the fd for input
    cmd->input_fd = fd;
    
    // Set input_file to NULL to ensure we use input_fd
    cmd->input_file = NULL;
    
    // Remove ALL heredoc delimiters from arguments
    if (cmd->args && cmd->redirections)
    {
        t_redirection *redir = cmd->redirections;
        while (redir)
        {
            if (redir->type == TOKEN_HEREDOC)
            {
                // Loop through all arguments to find and remove delimiter
                int i = 0;
                while (cmd->args[i])
                {
                    if (ft_strcmp(cmd->args[i], redir->word) == 0)
                    {
                        free(cmd->args[i]);
                        
                        // Shift remaining arguments
                        int j = i;
                        while (cmd->args[j + 1])
                        {
                            cmd->args[j] = cmd->args[j + 1];
                            j++;
                        }
                        cmd->args[j] = NULL;
                        i--; // Recheck this position since we shifted
                    }
                    i++;
                }
            }
            redir = redir->next;
        }
    }
    
    return (1);
}

int setup_redirections(t_cmd *cmd)
{
    // Handle input redirection - ENSURE input_fd takes precedence
    if (cmd->input_fd != -1)
    {
        // We already have an open file descriptor
        if (dup2(cmd->input_fd, STDIN_FILENO) == -1)
        {
            display_error(ERR_REDIR, "input", strerror(errno));
            return (1);
        }
    }
    else if (cmd->input_file)
    {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd == -1)
        {
            display_error(ERR_REDIR, cmd->input_file, strerror(errno));
            return (1);
        }
        
        if (dup2(fd, STDIN_FILENO) == -1)
        {
            close(fd);
            display_error(ERR_REDIR, cmd->input_file, strerror(errno));
            return (1);
        }
        
        close(fd);
    }
    
    // Handle output redirection
    if (cmd->output_file)
    {
        int flags = O_WRONLY | O_CREAT;
        if (cmd->append_mode)
            flags |= O_APPEND;
        else
            flags |= O_TRUNC;
            
        int fd = open(cmd->output_file, flags, 0644);
        if (fd == -1)
        {
            display_error(ERR_REDIR, cmd->output_file, strerror(errno));
            return (1);
        }
        
        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            close(fd);
            display_error(ERR_REDIR, cmd->output_file, strerror(errno));
            return (1);
        }
        
        close(fd);
    }
    
    return (0);
}
