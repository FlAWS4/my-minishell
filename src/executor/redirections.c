/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 22:11:44 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


/**
 * Check if a redirection's delimiter is empty (for heredocs)
 * This should check before any expansion
 */
int is_empty_delimiter(char *word)
{
    // Simply check for NULL or empty string
    return (!word || !*word);
}
    
/**
 * Create a temporary file for heredoc content
 */
static char *create_heredoc_tempfile(int *fd_out)
{
    char *temp_file;
    char *pid_str;
    char *result;
    
    temp_file = ft_strdup("/tmp/minishell_heredoc_");
    if (!temp_file)
        return (NULL);
        
    pid_str = ft_itoa(getpid());
    if (!pid_str)
    {
        free(temp_file);
        return (NULL);
    }
    
    result = ft_strjoin(temp_file, pid_str);
    free(temp_file);
    free(pid_str);
    
    if (!result)
        return (NULL);
        
    *fd_out = open(result, O_CREAT | O_RDWR | O_TRUNC, 0600);
    if (*fd_out == -1)
    {
        free(result);
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (NULL);
    }
    
    return (result);
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
    
    // Special handling for empty delimiters
    if (!delimiter || !*delimiter || fd < 0)
    {
        // Empty delimiter - don't prompt for input at all
        exit(0);
    }
    
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
            else
            {
                display_error(ERR_MEMORY, "heredoc", "variable expansion failed");
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
    {
        display_error(ERR_FORK, "heredoc", strerror(errno));
        return (0);
    }  
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
    if (WIFEXITED(status) && WEXITSTATUS(status) == 1)
    {
        // EOF was reached (Ctrl+D), but we should consider this valid
        return (1);
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
    char    *clean_delim;
    
    if (!redir || !redir->word)
        return (1);
        
    temp_file = create_heredoc_tempfile(&fd);
    if (!temp_file)
        return (1);
    
    clean_delim = ft_strdup(redir->word);
    if (!clean_delim)
    {
        close(fd);
        unlink(temp_file);
        free(temp_file);
        return (1);
    }
    
    if (!collect_heredoc_input(clean_delim, fd, redir->quoted, shell))
    {
        free(clean_delim);
        close(fd);
        unlink(temp_file);
        free(temp_file);
        return (1);
    }
    
    free(clean_delim);
    close(fd);
    
    redir->input_fd = open(temp_file, O_RDONLY);
    if (redir->input_fd == -1)
    {
        unlink(temp_file);
        free(temp_file);
        return (1);
    }
    
    redir->temp_file = temp_file;
    return (0);
}


int process_heredoc(t_cmd *cmd, t_shell *shell)
{
    t_redirection *redir;
    int success = 1;
    int last_fd = -1;

    if (cmd->heredocs_processed || g_signal == SIGINT)
        return (1);
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_HEREDOC)
        {
            if (process_heredoc_redir(redir, shell) != 0)
            {
                if (g_signal == SIGINT)
                    return (0);
                success = 0;  // Mark as failed but continue processing
            }
            else
                last_fd = redir->input_fd;
        }
        redir = redir->next;
    }
    if (last_fd != -1)
        cmd->input_fd = last_fd;
    setup_signals();
    cmd->heredocs_processed = 1;
    return success;
}

/**
 * Process regular file redirections only (not heredocs)
 */
int process_regular_redirections(t_cmd *cmd)
{
    t_redirection *redir;
    int flags;
    
    if (!cmd)
        return (0);
    
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_REDIR_IN)
        {
            redir->input_fd = open(redir->word, O_RDONLY);
            if (redir->input_fd == -1)
            {
                if (access(redir->word, F_OK) == -1)
                    display_error(ERR_REDIR, redir->word, "No such file or directory");
                else
                    display_error(ERR_REDIR, redir->word, strerror(errno));
                cleanup_redirections(cmd);
                return (1);
            }
        }
        else if (redir->type == TOKEN_REDIR_OUT || redir->type == TOKEN_REDIR_APPEND)
        {
            if (redir->type == TOKEN_REDIR_OUT)
                flags = O_WRONLY | O_CREAT | O_TRUNC;
            else
                flags = O_WRONLY | O_CREAT | O_APPEND;
                      
            redir->output_fd = open(redir->word, flags, 0644);
            if (redir->output_fd == -1)
            {
                display_error(ERR_REDIR, redir->word, strerror(errno));
                cleanup_redirections(cmd);
                return (1);
            }
        }
        redir = redir->next;
    }
    
    return (0);
}

/**
 * Process redirections, handling heredocs and regular files
 */
int process_redirections(t_cmd *cmd, t_shell *shell)
{
    // Process all heredocs first
    if (!cmd->heredocs_processed)
    {
        if (!process_heredoc(cmd, shell) && g_signal == SIGINT)
            return (1);
    }
    
    // Then process regular redirections
    return process_regular_redirections(cmd);
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
        
    // Find last input and output in a single pass
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
    
    // Apply input redirection if needed
    if (last_in != -1)
    {
        if (dup2(last_in, STDIN_FILENO) == -1)
        {
            display_error(ERR_REDIR, "input", strerror(errno));
            close(last_in);
            if (last_out != -1)
                close(last_out);
            return (1);
        }
        close(last_in);
    }
    
    // Apply output redirection if needed
    if (last_out != -1)
    {
        if (dup2(last_out, STDOUT_FILENO) == -1)
        {
            display_error(ERR_REDIR, "output", strerror(errno));
            close(last_out);
            return (1);
        }
        close(last_out);
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
            unlink(redir->temp_file);
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
 * Add a redirection to a command
 * Returns 1 on success, 0 on failure
 */
int add_redirection(t_cmd *cmd, int type, char *word, int quoted)
{
    t_redirection *new_redir;
    t_redirection *last;
    
    if (!cmd || !word)
        return (0);
        
    new_redir = malloc(sizeof(t_redirection));
    if (!new_redir)
        return (0);
        
    // Initialize the new redirection
    new_redir->type = type;
    new_redir->word = word;
    new_redir->quoted = quoted;
    new_redir->next = NULL;
    new_redir->input_fd = -1;
    new_redir->output_fd = -1;
    new_redir->temp_file = NULL;
    
    // Add to the end of the redirection list
    if (!cmd->redirections)
    {
        cmd->redirections = new_redir;
    }
    else
    {
        last = cmd->redirections;
        while (last->next)
            last = last->next;
        last->next = new_redir;
    }
    
    // If it's a heredoc, save the delimiter
    if (type == TOKEN_HEREDOC)
        cmd->heredoc_delim = ft_strdup(word);
        
    return (1);
}

/**
 * Setup all redirections (unified function)
 */
int setup_redirections(t_cmd *cmd, t_shell *shell)
{
    // Simplified early return check
    if (!cmd || !cmd->redirections)
        return (0);
        
    if (process_redirections(cmd, shell) != 0)
        return (1);
    if (apply_redirections(cmd) != 0)
        return (1);
        
    return (0);
}
