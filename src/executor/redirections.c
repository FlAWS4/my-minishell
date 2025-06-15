/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/15 09:27:09 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create a temporary file for heredoc content
 */
static char *create_heredoc_tempfile(int *fd_out)
{
    char    *temp_file;
    char    *pid_str;
    
    temp_file = ft_strdup("/tmp/minishell_heredoc_");
    if (!temp_file)
        return (NULL);
    
    pid_str = ft_itoa(getpid());
    if (!pid_str)
    {
        free(temp_file);
        return (NULL);
    }
    
    temp_file = ft_strjoin_free(temp_file, pid_str);
    free(pid_str);
    
    *fd_out = open(temp_file, O_CREAT | O_RDWR | O_TRUNC, 0600);
    if (*fd_out == -1)
    {
        free(temp_file);
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (NULL);
    }
    return (temp_file);
}

/**
 * Handle heredoc child process using readline
 */
static void handle_heredoc_child(char *delimiter, int fd, t_shell *shell, int quoted)
{
    char *line;
    char *expanded;
    
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_IGN);
    
    while (1)
    {
        line = readline("heredoc> ");
        if (!line)
        {
            display_heredoc_eof_warning(delimiter);
            exit(1); // Exit with code 1 to indicate EOF
        }
            
        if (ft_strcmp(line, delimiter) == 0)
        {
            free(line);
            break;
        }
        
        // Expand variables if not quoted
        if (!quoted && shell && ft_strchr(line, '$'))
        {
            expanded = expand_variables(shell, line);
            if (expanded)
            {
                free(line);
                line = expanded;
            }
        }
        
        ft_putstr_fd(line, fd);
        ft_putstr_fd("\n", fd);
        free(line);
    }
    
    exit(0);
}

/**
 * Collect input for a heredoc until delimiter is found
 */
int collect_heredoc_input(char *delimiter, int fd, int quoted, t_shell *shell)
{
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == -1)
        return (0);
        
    if (pid == 0)
    {
        // Child process that reads the heredoc input
        handle_heredoc_child(delimiter, fd, shell, quoted);
    }
        
    // Parent process waits for child
    waitpid(pid, &status, 0);
    
    // Simple signal reset
    setup_signals();
    
    // If child was terminated by signal (Ctrl+C)
    if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
    {
        g_signal = SIGINT;
        return (0);
    }
    
    // Return status from child to detect EOF properly
    if (WIFEXITED(status) && WEXITSTATUS(status) == 1)
    {
        // Child exited due to EOF - handle without executing command
        return (0);
    }
    
    return (1);
}

/**
 * Process a single heredoc redirection
 */

int process_heredoc_redir(t_redirection *redir, t_shell *shell)
{
    int     fd;
    char    *temp_file;
    char    *clean_delim = NULL;
    
    if (!redir || !redir->word)
        return (1);
        
    temp_file = create_heredoc_tempfile(&fd);
    if (!temp_file)
        return (1);
    
    redir->temp_file = temp_file;
    
    // Trust the quoted flag from the parser
    // Only clean up the delimiter if needed
    clean_delim = ft_strdup(redir->word);
    if (!clean_delim)
        clean_delim = redir->word; // Fallback if strdup fails
    
    // Use the clean delimiter for comparison
    if (!collect_heredoc_input(clean_delim, fd, redir->quoted, shell))
    {
        if (clean_delim != redir->word)
            free(clean_delim);
        close(fd);
        unlink(temp_file);
        return (1);
    }
    
    if (clean_delim != redir->word)
        free(clean_delim);
    
    close(fd);
    redir->input_fd = open(temp_file, O_RDONLY);
    if (redir->input_fd == -1)
    {
        unlink(temp_file);
        free(temp_file);
        redir->temp_file = NULL;
        return (1);
    }
    
    return (0);
}

/**
 * Process all heredocs in a command
 */
int process_heredoc(t_cmd *cmd, t_shell *shell)
{
    t_redirection *redir;
    int success = 1;
    int heredoc_count = 0;

    if (cmd->heredocs_processed || g_signal == SIGINT)
        return (1);
        
    // First count how many heredocs we have
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_HEREDOC)
            heredoc_count++;
        redir = redir->next;
    }
    
    // Now process each one
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_HEREDOC)
        {
            printf("Processing heredoc %d/%d with delimiter: %s\n", 
                   heredoc_count - 1, heredoc_count, redir->word);
                   
            if (process_heredoc_redir(redir, shell) != 0)
            {
                if (g_signal == SIGINT)
                    return (0);
                success = 0;  // Mark as failed but continue processing
            }
        }
        redir = redir->next;
    }
    
    cmd->heredocs_processed = 1;
    return success;
}

/**
 * Process regular file redirections
 */
int process_redirections(t_cmd *cmd, t_shell *shell)
{
    t_redirection *redir;
    
    if (!cmd)
        return (0);
        
    // First process heredocs if needed
    if (!cmd->heredocs_processed)
    {
        // Change this line to handle heredoc EOF more gracefully
        if (!process_heredoc(cmd, shell) && g_signal == SIGINT)
            return (1);  // Only abort if interrupted by SIGINT
    }
    
    // Then handle regular redirections
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_REDIR_IN)
        {
            redir->input_fd = open(redir->word, O_RDONLY);
            if (redir->input_fd == -1)
            {
                display_error(ERR_REDIR, redir->word, strerror(errno));
                return (1);
            }
        }
        else if (redir->type == TOKEN_REDIR_OUT || redir->type == TOKEN_REDIR_APPEND)
        {
            int flags = (redir->type == TOKEN_REDIR_OUT) 
                      ? O_WRONLY | O_CREAT | O_TRUNC 
                      : O_WRONLY | O_CREAT | O_APPEND;
                      
            redir->output_fd = open(redir->word, flags, 0644);
            if (redir->output_fd == -1)
            {
                display_error(ERR_REDIR, redir->word, strerror(errno));
                return (1);
            }
        }
        redir = redir->next;
    }
    
    return (0);
}

/**
 * Apply redirections to stdin/stdout
 */
int apply_redirections(t_cmd *cmd)
{
    t_redirection *redir;
    int last_in = -1, last_out = -1;
    
    if (!cmd)
        return (0);
        
    // Find last input and output
    redir = cmd->redirections;
    while (redir)
    {
        if ((redir->type == TOKEN_REDIR_IN || redir->type == TOKEN_HEREDOC) 
            && redir->input_fd != -1)
            last_in = redir->input_fd;
        else if ((redir->type == TOKEN_REDIR_OUT || redir->type == TOKEN_REDIR_APPEND) 
                && redir->output_fd != -1)
            last_out = redir->output_fd;
        redir = redir->next;
    }
    
    // Apply redirections - only modify what's needed
    if (last_in != -1)
    {
        if (dup2(last_in, STDIN_FILENO) == -1)
        {
            display_error(ERR_REDIR, "input", strerror(errno));
            return (1);
        }
        close(last_in); // Close after duplicate
    }
    
    if (last_out != -1)
    {
        if (dup2(last_out, STDOUT_FILENO) == -1)
        {
            display_error(ERR_REDIR, "output", strerror(errno));
            return (1);
        }
        close(last_out); // Close after duplicate
    }
    
    return (0);
}

/**
 * Clean up file descriptors
 */
void cleanup_redirections(t_cmd *cmd)
{
    t_redirection *redir;
    
    if (!cmd)
        return;
        
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->input_fd != -1)
        {
            close(redir->input_fd);
            redir->input_fd = -1;
        }
        if (redir->output_fd != -1)
        {
            close(redir->output_fd);
            redir->output_fd = -1;
        }
        if (redir->temp_file)
        {
            unlink(redir->temp_file); // Make sure we clean up the file
            free(redir->temp_file);
            redir->temp_file = NULL;
        }
        redir = redir->next;
    }
    
    if (cmd->input_fd != -1)
    {
        close(cmd->input_fd);
        cmd->input_fd = -1;
    }
}

/**
 * Setup all redirections (unified function)
 */
int setup_redirections(t_cmd *cmd, t_shell *shell)
{
    // Early return for no redirections
    if (!cmd || (!cmd->redirections && cmd->input_fd == -1 && !cmd->input_file && !cmd->heredoc_delim))
        return (0);
        
    if (process_redirections(cmd, shell) != 0)
        return (1);
    if (apply_redirections(cmd) != 0)
        return (1);
        
    return (0);
}
