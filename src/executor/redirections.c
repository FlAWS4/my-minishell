/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/09 00:19:58 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create a temporary file for heredoc content
 */
static char *create_heredoc_tempfile(int *fd_out)
{
    char *temp_file;
    
    temp_file = ft_strdup("/tmp/minishell_heredoc_XXXXXX");
    if (!temp_file)
        return (NULL);
    
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
 * Execute a command and capture its output
 * Used for command substitution in heredocs
 */
static char *execute_subcommand(char *cmd, t_shell *shell)
{
    int     pipes[2];
    pid_t   pid;
    int     status;
    char    buffer[4096] = {0};
    char    *result;
    int     bytes_read;
    
    if (pipe(pipes) == -1)
        return (ft_strdup(""));
    
    pid = fork();
    if (pid == -1)
    {
        close(pipes[0]);
        close(pipes[1]);
        return (ft_strdup(""));
    }
    
    if (pid == 0)
    {
        close(pipes[0]);
        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[1]);
        
        // Convert env linked list to array for execve
        char **env_array = env_to_array(shell->env);
        if (!env_array)
            exit(127);
            
        char *args[] = {"/bin/sh", "-c", cmd, NULL};
        execve("/bin/sh", args, env_array);
        
        // Free memory if execve fails
        free_env_array(env_array);
        exit(127);
    }
    
    close(pipes[1]);
    bytes_read = read(pipes[0], buffer, 4095);
    close(pipes[0]);
    
    waitpid(pid, &status, 0);
    
    if (bytes_read > 0 && buffer[bytes_read - 1] == '\n')
        buffer[bytes_read - 1] = '\0';
    
    result = ft_strdup(buffer);
    if (!result)
        return (ft_strdup(""));
    
    return (result);
}

/**
 * Replace a substring in a string with another string
 */
static char *replace_substring(char *str, int start, int end, char *replacement)
{
    char *before;
    char *after;
    char *temp;
    char *result;
    
    before = ft_substr(str, 0, start);
    if (!before)
        return (NULL);
    
    after = ft_strdup(str + end);
    if (!after)
    {
        free(before);
        return (NULL);
    }
    
    temp = ft_strjoin(before, replacement);
    if (!temp)
    {
        free(before);
        free(after);
        return (NULL);
    }
    
    result = ft_strjoin(temp, after);
    
    free(before);
    free(after);
    free(temp);
    
    return (result);
}

/**
 * Expand command substitution in a string
 * Handles $(command) syntax
 */
char *expand_command_substitution(char *input, t_shell *shell)
{
    char *result;
    char *cmd_start;
    char *cmd;
    char *output;
    char *new_result;
    int i, j, paren_count;
    
    if (!input || !shell)
    {
        if (input)
            return (ft_strdup(input));
        else
            return (ft_strdup(""));
    }
    
    result = ft_strdup(input);
    if (!result)
        return (NULL);
    
    // Rest of the function remains the same
    
    i = 0;
    while (result[i])
    {
        if (result[i] == '$' && result[i+1] == '(')
        {
            cmd_start = &result[i];
            paren_count = 1;
            j = i + 2;
            
            // Find closing parenthesis, handling nested ones
            while (result[j] && paren_count > 0)
            {
                if (result[j] == '(')
                    paren_count++;
                else if (result[j] == ')')
                    paren_count--;
                j++;
            }
            
            if (paren_count == 0)
            {
                // Extract command
                cmd = ft_substr(result, i+2, j-i-3);
                if (!cmd)
                    break;
                
                // Execute command and get output
                output = execute_subcommand(cmd, shell);
                free(cmd);
                
                if (!output)
                    break;
                
                // Replace $(cmd) with output
                new_result = replace_substring(result, i, j, output);
                free(output);
                
                if (!new_result)
                    break;
                
                free(result);
                result = new_result;
                
                // Don't increment i - we need to check the new content
                // for nested substitutions or new substitutions from the replacement
            }
            else
                i++;
        }
        else
            i++;
    }
    
    return result;
}

/**
 * Collect input for heredoc until delimiter is encountered
 * Performs variable and command substitution for unquoted heredocs
 */
int collect_heredoc_input(char *delimiter, int fd, int quoted, t_shell *shell)
{
    char *line;
    char *expanded;
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == -1)
        return (0);
    
    if (pid == 0) 
    {
        setup_signals_heredoc();
        gnl_cleanup(STDIN_FILENO);
        
        ft_putstr_fd("heredoc> ", STDOUT_FILENO);
        
        while (1)
        {
            line = get_next_line(STDIN_FILENO);
            
            if (!line)
                exit(0);
            
            // Remove trailing newline
            int len = ft_strlen(line);
            if (len > 0 && line[len - 1] == '\n')
                line[len - 1] = '\0';
            
            // Check for delimiter
            if (ft_strcmp(line, delimiter) == 0)
            {
                free(line);
                exit(0);
            }
            
            // Perform expansions for unquoted heredocs only
             // In the child process section:
            if (!quoted && shell)
            {
               //Expand variables
                if (ft_strchr(line, '$'))
                {
                    // Correct argument order
                    expanded = expand_variables(shell, line);
                    if (expanded)
                    {
                        free(line);
                        line = expanded;
                    }
                }
        
                // Expand command substitution
                if (ft_strstr(line, "$("))
                {
                     expanded = expand_command_substitution(line, shell);
                    if (expanded)
                    {
                        free(line);
                        line = expanded;
                    }
                }   
            }            
            // Write to file
            ft_putstr_fd(line, fd);
            ft_putstr_fd("\n", fd);
            
            free(line);
            ft_putstr_fd("heredoc> ", STDOUT_FILENO);
        }
    }
    
    // Parent process waits and handles signals
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
int process_heredoc(t_cmd *cmd, t_shell *shell)
{
    int fd;
    char *temp_file;
    
    if (!cmd || !cmd->heredoc_delim)
        return (1);
    
    printf("DEBUG: Processing heredoc with delimiter: %s\n", cmd->heredoc_delim);
    
    temp_file = create_heredoc_tempfile(&fd);
    if (!temp_file)
        return (0);
    
    cmd->heredoc_file = temp_file;
    
    // Command-level heredocs are always unquoted (quoted=0)
    if (!collect_heredoc_input(cmd->heredoc_delim, fd, 0, shell))
    {
        close(fd);
        unlink(temp_file);
        return (0);
    }
    
    close(fd);
    fd = open(temp_file, O_RDONLY);
    if (fd == -1)
    {
        unlink(temp_file);
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (0);
    }
    
    printf("DEBUG: Heredoc processed, input_fd set to: %d\n", fd);
    cmd->input_fd = fd;
    unlink(temp_file);
    
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
    
    redir->output_fd = fd;
    return (0);
}

/**
 * Process heredoc redirection in a redirection list
 */
int process_heredoc_redir(t_redirection *redir, t_shell *shell)
{
    int fd;
    char *temp_file;
    
    if (!redir || !redir->word)
        return (1);
    
    printf("DEBUG: Processing heredoc redirection with delimiter: %s (quoted: %d)\n", 
           redir->word, redir->quoted);
    
    temp_file = create_heredoc_tempfile(&fd);
    if (!temp_file)
        return (1);
    
    redir->temp_file = temp_file;
    
    // Pass the quoted flag to determine if expansion should happen
    if (!collect_heredoc_input(redir->word, fd, redir->quoted, shell))
    {
        close(fd);
        unlink(temp_file);
        free(temp_file);
        redir->temp_file = NULL;
        return (1);
    }
    
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
    
    redir->input_fd = fd;
    unlink(temp_file);
    
    return (0);
}

/**
 * Process a single redirection based on type
 * For heredoc redirections, pass the shell context
 */
int process_single_redir(t_redirection *redir, t_shell *shell)
{
    if (!redir)
        return (0);
        
    if (redir->type == TOKEN_REDIR_IN)
        return (process_input_redir(redir));
    else if (redir->type == TOKEN_REDIR_OUT || 
            redir->type == TOKEN_REDIR_APPEND)
        return (process_output_redir(redir));
    else if (redir->type == TOKEN_HEREDOC)
        return (process_heredoc_redir(redir, shell));
    
    return (0);
}

/**
 * Process all redirections in a command's redirection list
 */
int process_redirections(t_cmd *cmd, t_shell *shell)
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
                    if (process_heredoc_redir(current, shell) != 0)
                        return (1);
                }
                current = current->next;
            }
        }
        
        // THEN process command-level heredoc
        if (cmd->heredoc_delim && cmd->input_fd == -1)
        {
            printf("DEBUG: Processing command-level heredoc\n");
            if (!process_heredoc(cmd, shell))
                return (1);
        }
    }
    else
    {
        printf("DEBUG: Skipping heredoc processing - already processed\n");
    }
    
    // Process other redirections
    if (cmd->redirections)
    {
        current = cmd->redirections;
        while (current)
        {
            if (current->type != TOKEN_HEREDOC)
            {
                if (process_single_redir(current, shell) != 0)
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
int setup_redirections(t_cmd *cmd, t_shell *shell)
{
    printf("DEBUG: Setting up redirections\n");
    
    // First process all redirections to set up file descriptors
    if (process_redirections(cmd, shell) != 0)
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
                    if (process_heredoc_redir(redir, shell) != 0)
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
            if (!process_heredoc(current, shell))
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
