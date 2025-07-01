/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/24 01:43:24 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * combine_command_arguments - Joins multiple command arguments into a single string
 * @shell: Shell context containing garbage collection
 * @args: Array of string arguments to join
 * 
 * This function concatenates all command arguments into a single string,
 * primarily used for error reporting and debug output. It handles memory
 * management through the shell's garbage collection system.
 * 
 * Returns: A newly allocated string containing all arguments joined together
 */
char	*combine_command_arguments(t_shell *shell, char **args)
{
    char	*result;
    char	*tmp;
    int		i;

    i = 0;
    if (!args || !args[0])
        return (create_managed_string_copy(&shell->memory_manager, ""));
    result = create_managed_string_copy(&shell->memory_manager, args[i]);
    i++;
    while (args[i])
    {
        tmp = join_managed_strings(&shell->memory_manager, result, "");
        result = join_managed_strings(&shell->memory_manager, tmp, args[i]);
        i++;
    }
    return (result);
}

/**
 * report_file_error - Reports file operation error based on errno
 * @filename: Name of file that caused the error
 * 
 * This function determines the appropriate error message based on the
 * system error code (errno) and reports it to the user. It handles
 * common file operation errors with specific messages.
 * 
 * Returns: -1 to indicate an error occurred
 */
int	report_file_error(const char *filename)
{
    if (errno == EISDIR)
        error(NULL, filename, "Is a directory");
    else if (errno == EACCES)
        error(NULL, filename, "Permission denied");
    else
        error(NULL, filename, strerror(errno));
    return (-1);
}

/**
 * open_redirection_target - Opens a file or sets up a heredoc for redirection
 * @redir: Redirection structure containing type and target information
 * @cmd: Command structure to update with the opened file descriptor
 * 
 * This function handles opening files or setting up pipes for all redirection
 * types (input, output, append, heredoc). It checks for ambiguous redirections
 * and empty filenames, reporting appropriate errors.
 * 
 * Returns: File descriptor on success, -1 on failure, 0 for heredocs
 */
int	open_redirection_target(t_redir *redir, t_command *cmd)
{
    if (redir->ar)
        return (-1);
     if (redir->type == HEREDOC)
    {
        if (setup_heredoc_pipe(cmd, redir) == -1)
            return (-1);
        return (0);
    }
    if (!redir->file_or_del || !*redir->file_or_del)
    {
        error(NULL, "", "No such file or directory\n");
        return (-1);
    }
    if (redir->type == REDIR_IN)
        return (open_file_for_input(redir->file_or_del));
    else if (redir->type == REDIR_OUT)
        return (open_file_for_output(redir->file_or_del));
    else if (redir->type == APPEND)
        return (open_file_for_append(redir->file_or_del));
    else if (redir->type == HEREDOC)
    {
        if (setup_heredoc_pipe(cmd, redir) == -1)
            return (-1);
        return (0);
    }
    return (-1);
}

/**
 * process_command_redirections - Processes all redirections for a command
 * @cmd: Command structure containing redirections to process
 * @shell: Shell context for error handling and resource management
 * 
 * This function iterates through all redirections for a command,
 * opening files as needed and updating the command's file descriptors.
 * If any redirection fails, it cleans up resources and returns an error.
 * 
 * Returns: 0 on success, -1 on failure
 */
int	process_command_redirections(t_command *cmd, t_shell *shell)
{
    t_redir	*redir;
    int		fd;

    redir = cmd->redirs;
    while (redir)
    {
        fd = open_redirection_target(redir, cmd);
        if (fd == -1)
        {
            cleanup_shell_file_descriptors(shell);
            g_exit_status = 1;
            return (-1);
        }
        if (redir->type != HEREDOC || fd > 0)
            update_command_redirections(cmd, redir->type, fd);
        redir = redir->next;
    }
    return (0);
}

/*
 * execute_builtin_with_redirections - Runs a builtin with I/O redirections
 * @shell: Shell context containing environment and builtins
 * @cmd: Command structure with arguments and redirections
 * 
 * This function temporarily applies redirections for a builtin command,
 * saving the original stdin/stdout, executing the builtin, and then
 * restoring the original I/O. This allows builtins to work with
 * redirections without affecting the shell's main I/O.
 * 
 * Returns: Exit status of the builtin command

int	execute_builtin_with_redirections(t_shell *shell, t_command *cmd)
{
    int	saved_stdin;
    int	saved_stdout;
    int	result;

    saved_stdin = -1;
    saved_stdout = -1;
    if (cmd->fd_in != STDIN_FILENO && cmd->fd_in != -1)
        saved_stdin = dup(STDIN_FILENO);
    if (cmd->fd_out != STDOUT_FILENO && cmd->fd_out != -1)
        saved_stdout = dup(STDOUT_FILENO);
    apply_command_redirections(cmd);
    result = run_builtin(shell, cmd);
    if (saved_stdin != -1)
    {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    if (saved_stdout != -1)
    {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
    return (result);
}
*/