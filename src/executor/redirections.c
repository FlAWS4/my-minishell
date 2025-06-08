/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/08 22:46:28 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create a temporary file for heredoc content
 */
static char *create_heredoc_tempfile(int *fd_out)
{
    char *temp_file;
    
    // Create unique temp filename
    temp_file = ft_strdup("/tmp/minishell_heredoc_XXXXXX");
    if (!temp_file)
        return (NULL);
    
    // Create the file with a unique name
    *fd_out = mkstemp(temp_file);
    if (*fd_out == -1)
    {
        free(temp_file);
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (NULL);
    }
    
    return temp_file;
}

/**
 * Collect input for heredoc until delimiter is encountered
 * Handles SIGINT properly to allow cancellation
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
        gnl_cleanup(STDIN_FILENO);
        
        // Prompt for input
        ft_putstr_fd("heredoc> ", STDOUT_FILENO);
        
        while (1)
        {
            line = get_next_line(STDIN_FILENO);
            
            // Check for EOF or error
            if (!line)
                exit(0);
            
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
    
    // Parent process - wait with proper signal handling
    signal(SIGINT, SIG_IGN);
    waitpid(pid, &status, 0);
    setup_signals();
    
    if (WIFSIGNALED(status))
    {
        g_signal = SIGINT;
        return (0);
    }
    
    return (1);
}

/**
 * Process a heredoc for a command
 */
int process_heredoc(t_cmd *cmd)
{
    int fd;
    char *temp_file;
    
    if (!cmd || !cmd->heredoc_delim)
        return (1);
    
    printf("DEBUG: Processing heredoc with delimiter: %s\n", cmd->heredoc_delim);
    
    // Create temp file
    temp_file = create_heredoc_tempfile(&fd);
    if (!temp_file)
        return (0);
    
    // Store the filename for later cleanup
    cmd->heredoc_file = temp_file;
    
    // Collect the content
    if (!collect_heredoc_input(cmd->heredoc_delim, fd))
    {
        close(fd);
        unlink(temp_file);
        return (0);
    }
    
    // Close write end and reopen for reading
    close(fd);
    fd = open(temp_file, O_RDONLY);
    if (fd == -1)
    {
        unlink(temp_file);
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (0);
    }
    
    // Store fd for input redirection and mark file for cleanup
    printf("DEBUG: Heredoc processed, input_fd set to: %d\n", fd);
    cmd->input_fd = fd;
    unlink(temp_file); // Remove file, fd stays valid
    
    return (1);
}

/**
 * Process input redirection in a redirection list
 */
int process_input_redir(t_redirection *redir)
{
    int fd;
    
    if (!redir || !redir->word)
        return (1);
    
    printf("DEBUG: Processing input redirection: %s\n", redir->word);
    
    fd = open(redir->word, O_RDONLY);
    if (fd == -1)
    {
        display_error(ERR_REDIR, redir->word, strerror(errno));
        return (1);
    }
    
    // Store fd instead of applying immediately
    redir->input_fd = fd;
    return (0);
}

/**
 * Process output redirection in a redirection list
 */
int process_output_redir(t_redirection *redir)
{
    int fd;
    int flags;
    
    if (!redir || !redir->word)
        return (1);
    
    printf("DEBUG: Processing output redirection: %s\n", redir->word);
    
    if (redir->type == TOKEN_REDIR_OUT)
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    else // TOKEN_REDIR_APPEND
        flags = O_WRONLY | O_CREAT | O_APPEND;
    
    fd = open(redir->word, flags, 0644);
    if (fd == -1)
    {
        display_error(ERR_REDIR, redir->word, strerror(errno));
        return (1);
    }
    
    // Store in output_fd
    redir->output_fd = fd;
    return (0);
}

/**
 * Process heredoc redirection in a redirection list
 */
int process_heredoc_redir(t_redirection *redir)
{
    int fd;
    char *temp_file;
    
    if (!redir || !redir->word)
        return (1);
    
    printf("DEBUG: Processing heredoc redirection with delimiter: %s\n", redir->word);
    
    // Create temp file
    temp_file = create_heredoc_tempfile(&fd);
    if (!temp_file)
        return (1);
    
    // Store temp file name
    redir->temp_file = temp_file;
    
    // Collect heredoc content
    if (!collect_heredoc_input(redir->word, fd))
    {
        close(fd);
        unlink(temp_file);
        free(temp_file);
        redir->temp_file = NULL;
        return (1);
    }
    
    // Close write end and reopen for reading
    close(fd);
    fd = open(temp_file, O_RDONLY);
    if (fd == -1)
    {
        unlink(temp_file);
        free(temp_file);
        redir->temp_file = NULL;
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    
    // Store file descriptor and remove the file (fd stays valid)
    redir->input_fd = fd;
    unlink(temp_file);
    
    return (0);
}

/**
 * Process a single redirection based on type
 */
int process_single_redir(t_redirection *redir)
{
    if (!redir)
        return (0);
        
    if (redir->type == TOKEN_REDIR_IN)
        return (process_input_redir(redir));
    else if (redir->type == TOKEN_REDIR_OUT || 
            redir->type == TOKEN_REDIR_APPEND)
        return (process_output_redir(redir));
    else if (redir->type == TOKEN_HEREDOC)
        return (process_heredoc_redir(redir));
    return (0);
}

/**
 * Process all redirections in a command's redirection list
 */
int process_redirections(t_cmd *cmd)
{
    t_redirection *current;
    
    if (!cmd)
        return (0);
        
    // Skip processing heredocs if already processed
    if (!cmd->heredocs_processed) 
    {
        // Process heredocs in redirection list FIRST
        if (cmd->redirections)
        {
            current = cmd->redirections;
            while (current)
            {
                if (current->type == TOKEN_HEREDOC)
                {
                    if (process_heredoc_redir(current) != 0)
                        return (1);
                }
                current = current->next;
            }
        }
        
        // THEN process command-level heredoc (which is actually the last one)
        if (cmd->heredoc_delim && cmd->input_fd == -1)
        {
            printf("DEBUG: Processing command-level heredoc\n");
            if (!process_heredoc(cmd))
                return (1);
        }
    }
    else
    {
        printf("DEBUG: Skipping heredoc processing - already processed\n");
    }
    
    // Always process other redirections
    if (cmd->redirections)
    {
        current = cmd->redirections;
        while (current)
        {
            if (current->type != TOKEN_HEREDOC)
            {
                if (process_single_redir(current) != 0)
                    return (1);
            }
            current = current->next;
        }
    }
    
    return (0);
}

/**
 * Apply redirections to stdin/stdout for a command
 */
int apply_redirections(t_cmd *cmd)
{
    t_redirection *redir;
    int last_input_fd = -1;
    int last_output_fd = -1;
    t_redirection *last_heredoc = NULL;
    
    if (!cmd)
        return (0);
    
    // Find the last heredoc in redirections list
    if (cmd->redirections)
    {
        redir = cmd->redirections;
        while (redir)
        {
            if (redir->type == TOKEN_HEREDOC && redir->input_fd != -1)
                last_heredoc = redir;
            
            if ((redir->type == TOKEN_REDIR_OUT || redir->type == TOKEN_REDIR_APPEND) &&
                redir->output_fd != -1)
            {
                last_output_fd = redir->output_fd;
            }
            
            redir = redir->next;
        }
    }
    
    // Use command-level heredoc only if it's the last one
    if (cmd->heredoc_delim && cmd->input_fd != -1)
    {
        // Command-level is the last one
        last_input_fd = cmd->input_fd;
    }
    else if (last_heredoc)
    {
        // Use the last heredoc from redirections
        last_input_fd = last_heredoc->input_fd;
    }
    
    // Apply regular input redirections (they override heredocs)
    if (cmd->redirections)
    {
        redir = cmd->redirections;
        while (redir)
        {
            if (redir->type == TOKEN_REDIR_IN && redir->input_fd != -1)
                last_input_fd = redir->input_fd;
            
            redir = redir->next;
        }
    }
    
    // Apply the last input redirection
    if (last_input_fd != -1)
    {
        printf("DEBUG: Applying last input redirection fd=%d\n", last_input_fd);
        if (dup2(last_input_fd, STDIN_FILENO) == -1)
        {
            display_error(ERR_REDIR, "input", strerror(errno));
            return (1);
        }
    }
    
    // Apply the last output redirection
    if (last_output_fd != -1)
    {
        printf("DEBUG: Applying last output redirection fd=%d\n", last_output_fd);
        if (dup2(last_output_fd, STDOUT_FILENO) == -1)
        {
            display_error(ERR_REDIR, "output", strerror(errno));
            return (1);
        }
    }
    
    return (0);
}


/**
 * Clean up file descriptors after command execution
 */
void cleanup_redirections(t_cmd *cmd)
{
    t_redirection *redir;
    
    if (!cmd)
        return;
    
    // Close command-level input_fd
    if (cmd->input_fd != -1)
    {
        printf("DEBUG: Closing command input_fd=%d\n", cmd->input_fd);
        close(cmd->input_fd);
        cmd->input_fd = -1;
    }
    
    // Free command-level heredoc file
    if (cmd->heredoc_file)
    {
        free(cmd->heredoc_file);
        cmd->heredoc_file = NULL;
    }
    
    if (!cmd->redirections)
        return;
    
    // Close all redirection file descriptors
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->input_fd != -1)
        {
            printf("DEBUG: Closing redirection input_fd=%d\n", redir->input_fd);
            close(redir->input_fd);
            redir->input_fd = -1;
        }
        
        if (redir->output_fd != -1)
        {
            printf("DEBUG: Closing redirection output_fd=%d\n", redir->output_fd);
            close(redir->output_fd);
            redir->output_fd = -1;
        }
        
        if (redir->temp_file)
        {
            free(redir->temp_file);
            redir->temp_file = NULL;
        }
        
        redir = redir->next;
    }
}

/**
 * Setup redirections for a command - unified function
 */
int setup_redirections(t_cmd *cmd)
{
    printf("DEBUG: Setting up redirections\n");
    
    // First process all redirections to set up file descriptors
    if (process_redirections(cmd) != 0)
        return (1);
    
    // Then apply them to stdin/stdout
    if (apply_redirections(cmd) != 0)
        return (1);
    
    return (0);
}

/**
 * Process all heredocs and then execute command(s)
 */
int process_and_execute_heredoc_command(t_shell *shell, t_cmd *cmd)
{
    t_cmd *current;
    int result;
    
    if (!shell || !cmd)
        return (1);
    
    // Process all heredocs first, but only once
    current = cmd;
    while (current)
    {
        // Mark command to prevent reprocessing heredocs
        current->heredocs_processed = 1;
        
        // Process heredocs in redirections list FIRST
        if (current->redirections)
        {
            t_redirection *redir = current->redirections;
            while (redir)
            {
                if (redir->type == TOKEN_HEREDOC)
                {
                    printf("DEBUG: Processing redirection heredoc with delimiter: %s\n", 
                           redir->word);
                    if (process_heredoc_redir(redir) != 0)
                        return (1);
                    
                    // If this is the same as command-level heredoc, clear the command-level one
                    // to avoid processing it twice
                    if (current->heredoc_delim && ft_strcmp(redir->word, current->heredoc_delim) == 0)
                    {
                        printf("DEBUG: Skipping duplicate command-level heredoc\n");
                        current->heredoc_delim = NULL;
                    }
                }
                redir = redir->next;
            }
        }
        
        // THEN process command-level heredoc (only if it wasn't already processed)
        if (current->heredoc_delim)
        {
            printf("DEBUG: Found command with heredoc delimiter: %s\n", 
                   current->heredoc_delim);
            if (!process_heredoc(current))
                return (1);
        }
        
        current = current->next;
    }
    
    // Execute command after all heredocs are processed
    if (cmd->args && cmd->args[0] && cmd->args[0][0])
    {
        printf("DEBUG: Executing command '%s' after processing heredocs\n", 
              cmd->args[0]);
        
        if (cmd->next)
            result = execute_pipeline(shell, cmd);
        else
            result = execute_command(shell, cmd);
            
        return (result);
    }
    else
    {
        printf("DEBUG: No command to execute after heredoc\n");
        return (0);
    }
}

/**
 * Check if command has any heredoc redirection
 */
int has_heredoc_redirection(t_cmd *cmd)
{
    t_redirection *redir;
    
    if (!cmd)
        return (0);
    
    // Check if there's a direct heredoc on the command
    if (cmd->heredoc_delim)
        return (1);
    
    // Check redirection list for heredocs
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_HEREDOC)
            return (1);
        redir = redir->next;
    }
    
    return (0);
}
