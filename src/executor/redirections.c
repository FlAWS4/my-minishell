/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/12 02:00:25 by mshariar         ###   ########.fr       */
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
    int     pid;
    
    pid = (int)getpid();
    temp_file = ft_strdup("/tmp/minishell_heredoc_");
    if (!temp_file)
        return (NULL);
    
    // Convert pid to string correctly
    pid_str = ft_itoa(pid);
    if (!pid_str)
    {
        free(temp_file);
        return (NULL);
    }
    
    // Append pid to filename
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
 * Handle child process for command substitution
 */
static void	handle_subcommand_child(t_shell *shell, char *cmd, int pipes[2])
{
    char	**env_array;
    char	*args[4];

    close(pipes[0]);
    dup2(pipes[1], STDOUT_FILENO);
    close(pipes[1]);
    env_array = env_to_array(shell->env);
    if (!env_array)
        exit(127);
    args[0] = "/bin/sh";
    args[1] = "-c";
    args[2] = cmd;
    args[3] = NULL;
    execve("/bin/sh", args, env_array);
    free_env_array(env_array);
    exit(127);
}

/**
 * Read command output from pipe
 */
static char	*read_command_output(int pipe_fd)
{
    char	buffer[4096];
    char	*result;
    int		bytes_read;

    ft_memset(buffer, 0, 4096);
    bytes_read = read(pipe_fd, buffer, 4095);
    if (bytes_read > 0 && buffer[bytes_read - 1] == '\n')
        buffer[bytes_read - 1] = '\0';
    result = ft_strdup(buffer);
    if (!result)
        return (ft_strdup(""));
    return (result);
}

/**
 * Execute a command and capture its output
 */
char	*execute_subcommand(char *cmd, t_shell *shell)
{
    int		pipes[2];
    pid_t	pid;
    int		status;
    char	*result;

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
        handle_subcommand_child(shell, cmd, pipes);
    close(pipes[1]);
    result = read_command_output(pipes[0]);
    close(pipes[0]);
    waitpid(pid, &status, 0);
    return (result);
}

/**
 * Replace a substring in a string with another string
 */
static char	*replace_substring(char *str, int start, int end, char *replacement)
{
    char	*before;
    char	*after;
    char	*temp;
    char	*result;

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
    return (free(before), free(after), free(temp), result);
}

/**
 * Find closing parenthesis for command substitution
 */
static int	find_closing_paren(char *str, int start)
{
    int	paren_count;
    int	j;

    paren_count = 1;
    j = start;
    while (str[j] && paren_count > 0)
    {
        if (str[j] == '(')
            paren_count++;
        else if (str[j] == ')')
            paren_count--;
        j++;
    }
    if (paren_count == 0)
        return (j);
    return (-1);
}

/**
 * Process a single command substitution
 */
static int	process_one_substitution(char **result, int i, t_shell *shell)
{
    char	*cmd;
    char	*output;
    char	*new_result;
    int		j;

    j = find_closing_paren(*result, i + 2);
    if (j == -1)
        return (i + 1);
    cmd = ft_substr(*result, i + 2, j - i - 3);
    if (!cmd)
        return (i + 1);
    output = execute_subcommand(cmd, shell);
    free(cmd);
    if (!output)
        return (i + 1);
    new_result = replace_substring(*result, i, j, output);
    free(output);
    if (!new_result)
        return (i + 1);
    free(*result);
    *result = new_result;
    return (i);
}

/**
 * Expand command substitution in a string
 */
char	*expand_command_substitution(char *input, t_shell *shell)
{
    char	*result;
    int		i;

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
    i = 0;
    while (result[i])
    {
        if (result[i] == '$' && result[i + 1] == '(')
            i = process_one_substitution(&result, i, shell);
        else
            i++;
    }
    return (result);
}

/**
 * Handle heredoc child process
 */
void handle_heredoc_child(char *delimiter, int fd, t_shell *shell, int quoted)
{
    char *line;
    char *expanded;
    char *expanded_for_comparison;
    
    setup_signals_heredoc();
    ft_putstr_fd("heredoc> ", 1);
    while (1)
    {
        line = get_next_line(STDIN_FILENO);
        if (!line)
        {
            display_heredoc_eof_warning(delimiter);
            break;
        }
        
        // Remove newline
        if (ft_strlen(line) > 0 && line[ft_strlen(line) - 1] == '\n')
            line[ft_strlen(line) - 1] = '\0';
        
        // Check literal match first
        if (ft_strcmp(line, delimiter) == 0)
        {
            free(line);
            break;
        }
        
        // Then check for expanded match when the line contains $
        if (ft_strchr(line, '$'))
        {
            expanded_for_comparison = expand_variables(shell, line);
            if (expanded_for_comparison)
            {
                if (ft_strcmp(expanded_for_comparison, delimiter) == 0)
                {
                    free(expanded_for_comparison);
                    free(line);
                    break;
                }
                free(expanded_for_comparison);
            }
        }
        
        // Expand variables in content if not quoted
        if (!quoted && shell)
        {
            expanded = expand_heredoc_content(shell, line);
            if (expanded)
            {
                free(line);
                line = expanded;
            }
        }
        
        ft_putstr_fd(line, fd);
        ft_putstr_fd("\n", fd);
        free(line);
        ft_putstr_fd("heredoc> ", 1);
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
    
    // Reset global signal flag before forking
    g_signal = 0;
    
    pid = fork();
    if (pid == -1)
        return (0);
    if (pid == 0)
        handle_heredoc_child(delimiter, fd, shell, quoted); // Pass quoted flag
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status))
    {
        g_signal = WTERMSIG(status);
        shell->exit_status = 128 + g_signal;
        return (0);
    }
    else if (WEXITSTATUS(status) == 130)
    {
        g_signal = SIGINT;
        shell->exit_status = 130;
        return (0);
    }
    return (1);
}

/**
 * Process a heredoc for a command
 */
int process_heredoc(t_cmd *cmd, t_shell *shell)
{
    t_redirection *redir;

    if (cmd->heredocs_processed || g_signal == SIGINT)
        return 1;
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_HEREDOC)
        {
            if (process_heredoc_redir(redir, shell) != 0)
                return 0;
        }
        redir = redir->next;
    }
    
    // Mark heredocs as processed
    cmd->heredocs_processed = 1;
    return 1;
}

/**
 * Process input redirection in a redirection list
 */
int	process_input_redir(t_redirection *redir)
{
    int	fd;

    if (!redir || !redir->word)
        return (1);
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
int	process_output_redir(t_redirection *redir)
{
    int	fd;
    int	flags;

    if (!redir || !redir->word)
        return (1);
    if (redir->type == TOKEN_REDIR_OUT)
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    else
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
 * Handle heredoc redirection errors
 */
static int	handle_heredoc_redir_error(int fd, char *file, t_redirection *redir)
{
    close(fd);
    unlink(file);
    free(file);
    redir->temp_file = NULL;
    return (1);
}

/**
 * Handle file open errors for heredoc
 */
static int	handle_open_error(char *file, t_redirection *redir)
{
    unlink(file);
    free(file);
    redir->temp_file = NULL;
    display_error(ERR_REDIR, "heredoc", strerror(errno));
    return (1);
}

/**
 * Process heredoc redirection in a redirection list
 */
int process_heredoc_redir(t_redirection *redir, t_shell *shell)
{
    int     fd;
    char    *temp_file;
    char    *expanded_delimiter = NULL;

    if (!redir || !redir->word)
        return (1);
        
    // Expand variables in delimiter if not quoted
    if (!redir->quoted && ft_strchr(redir->word, '$'))
    {
        expanded_delimiter = expand_variables(shell, redir->word);
        if (!expanded_delimiter)
            expanded_delimiter = ft_strdup(redir->word);
    }
    else
        expanded_delimiter = ft_strdup(redir->word);
        
    temp_file = create_heredoc_tempfile(&fd);
    if (!temp_file)
    {
        free(expanded_delimiter);
        return (1);
    }
    
    redir->temp_file = temp_file;
    
    // Pass the expanded delimiter to collect_heredoc_input
    if (!collect_heredoc_input(expanded_delimiter, fd, redir->quoted, shell))
    {
        free(expanded_delimiter);
        return (handle_heredoc_redir_error(fd, temp_file, redir));
    }
    
    free(expanded_delimiter);
    close(fd);
    fd = open(temp_file, O_RDONLY);
    if (fd == -1)
        return (handle_open_error(temp_file, redir));
        
    redir->input_fd = fd;
    unlink(temp_file);
    return (0);
}

/**
 * Process a single redirection based on type
 */
int	process_single_redir(t_redirection *redir, t_shell *shell)
{
    if (!redir)
        return (0);
    if (redir->type == TOKEN_REDIR_IN)
        return (process_input_redir(redir));
    else if (redir->type == TOKEN_REDIR_OUT
        || redir->type == TOKEN_REDIR_APPEND)
        return (process_output_redir(redir));
    else if (redir->type == TOKEN_HEREDOC)
        return (process_heredoc_redir(redir, shell));
    return (0);
}

/**
 * Process heredocs in a command's redirection list
 */
static int	process_heredoc_redirections(t_cmd *cmd, t_shell *shell)
{
    t_redirection	*current;

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
    return (0);
}

/**
 * Process non-heredoc redirections in a command
 */
static int	process_other_redirections(t_cmd *cmd, t_shell *shell)
{
    t_redirection	*current;

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
 * Process all redirections in a command's redirection list
 */
int	process_redirections(t_cmd *cmd, t_shell *shell)
{
    if (!cmd)
        return (0);
    if (!cmd->heredocs_processed)
    {
        if (process_heredoc_redirections(cmd, shell) != 0)
            return (1);
        if (cmd->heredoc_delim && cmd->input_fd == -1)
        {
            if (!process_heredoc(cmd, shell))
                return (1);
        }
    }
    return (process_other_redirections(cmd, shell));
}

/**
 * Find the last heredoc in redirections list
 */
static t_redirection	*find_last_heredoc(t_cmd *cmd, int *last_output_fd)
{
    t_redirection	*redir;
    t_redirection	*last_heredoc;

    last_heredoc = NULL;
    if (!cmd->redirections)
        return (NULL);
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_HEREDOC && redir->input_fd != -1)
            last_heredoc = redir;
        if ((redir->type == TOKEN_REDIR_OUT || redir->type == TOKEN_REDIR_APPEND)
            && redir->output_fd != -1)
            *last_output_fd = redir->output_fd;
        redir = redir->next;
    }
    return (last_heredoc);
}

/**
 * Find the last input redirection
 */
static int	find_last_input_fd(t_cmd *cmd, t_redirection *last_heredoc)
{
    t_redirection	*redir;
    int				last_input_fd;

    last_input_fd = -1;
    if (cmd->heredoc_delim && cmd->input_fd != -1)
        last_input_fd = cmd->input_fd;
    else if (last_heredoc)
        last_input_fd = last_heredoc->input_fd;
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
    return (last_input_fd);
}

/**
 * Apply input redirection to stdin
 */
static int	apply_input_redirection(int fd)
{
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        display_error(ERR_REDIR, "input", strerror(errno));
        return (1);
    }
    return (0);
}

/**
 * Apply output redirection to stdout
 */
static int	apply_output_redirection(int fd)
{
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        display_error(ERR_REDIR, "output", strerror(errno));
        return (1);
    }
    return (0);
}

/**
 * Apply redirections to stdin/stdout for a command
 */
int	apply_redirections(t_cmd *cmd)
{
    int				last_input_fd;
    int				last_output_fd;
    t_redirection	*last_heredoc;

    if (!cmd)
        return (0);
    last_output_fd = -1;
    last_heredoc = find_last_heredoc(cmd, &last_output_fd);
    last_input_fd = find_last_input_fd(cmd, last_heredoc);
    if (last_input_fd != -1)
        if (apply_input_redirection(last_input_fd))
            return (1);
    if (last_output_fd != -1)
        if (apply_output_redirection(last_output_fd))
            return (1);
    return (0);
}

/**
 * Close file descriptors in redirection list
 */
static void	cleanup_redirection_list(t_redirection *redir)
{
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
            free(redir->temp_file);
            redir->temp_file = NULL;
        }
        redir = redir->next;
    }
}

/**
 * Clean up file descriptors after command execution
 */
void	cleanup_redirections(t_cmd *cmd)
{
    if (!cmd)
        return ;
    if (cmd->input_fd != -1)
    {
        close(cmd->input_fd);
        cmd->input_fd = -1;
    }
    if (cmd->heredoc_file)
    {
        free(cmd->heredoc_file);
        cmd->heredoc_file = NULL;
    }
    if (cmd->redirections)
        cleanup_redirection_list(cmd->redirections);
}

/**
 * Setup redirections for a command - unified function
 */
int	setup_redirections(t_cmd *cmd, t_shell *shell)
{
    if (process_redirections(cmd, shell) != 0)
        return (1);
    if (apply_redirections(cmd) != 0)
        return (1);
    return (0);
}

/**
 * Process heredocs for a single command
 */
static int	process_command_heredocs(t_cmd *current, t_shell *shell)
{
    t_redirection	*redir;

    if (current->heredocs_processed || g_signal == SIGINT)
        return (1);
    
    // Mark as processed only after checking
    current->heredocs_processed = 1;
    
    if (current->redirections)
    {
        redir = current->redirections;
        while (redir)
        {
            if (redir->type == TOKEN_HEREDOC)
            {
                if (process_heredoc_redir(redir, shell) != 0)
                    return (1);
                if (current->heredoc_delim
                    && ft_strcmp(redir->word, current->heredoc_delim) == 0)
                    current->heredoc_delim = NULL;
            }
            redir = redir->next;
        }
    }
    if (current->heredoc_delim)
    {
        if (!process_heredoc(current, shell))
            return (1);
    }
    return (0);
}

/**
 * Process all heredocs and then execute command(s)
 */
int	process_and_execute_heredoc_command(t_shell *shell, t_cmd *cmd)
{
    t_cmd	*current;
    int		result;

    if (!shell || !cmd)
        return (1);
    current = cmd;
    while (current)
    {
        if (process_command_heredocs(current, shell) != 0)
            return (1);
        current = current->next;
    }
    if (cmd->args && cmd->args[0] && cmd->args[0][0])
    {
        if (cmd->next)
            result = execute_pipeline(shell, cmd);
        else
            result = execute_command(shell, cmd);
        return (result);
    }
    return (0);
}

/**
 * Check if command has any heredoc redirection
 */
int	has_heredoc_redirection(t_cmd *cmd)
{
    t_redirection	*redir;

    if (!cmd)
        return (0);
    if (cmd->heredoc_delim)
        return (1);
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_HEREDOC)
            return (1);
        redir = redir->next;
    }
    return (0);
}
