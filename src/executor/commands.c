/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:22:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/24 01:26:15 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Joins two strings and frees the first input string
 * 
 * @param input  First string (will be freed)
 * @param str    Second string (will be appended)
 * 
 * @return New joined string or NULL if memory allocation fails
 * 
 * This function performs a join operation while handling memory management
 * by freeing the first string regardless of success or failure.
 */
static char	*join_strings_and_free_first(char *input, char *str)
{
    char	*s;

    s = ft_strjoin(input, str);
    if (!s)
        return (free(input), NULL);
    return (free(input), s);
}

/**
 * Appends text to an existing command input
 * 
 * @param input  Pointer to current command string (may be modified)
 * @param str    String to append (will be freed)
 * @param code   Status code to return after operation
 * 
 * @return Status code (0=complete, 1=error, 3=continue with pipe)
 * 
 * Joins the strings, handles memory allocation errors,
 * and updates the input pointer to contain the new combined string.
 */
static int	append_to_command_input(char **input, char *str, int code)
{
    char	*new_imput;

    new_imput = join_strings_and_free_first(*input, str);
    if (!new_imput)
        return (free(str), free(*input), *input = NULL, 1);
    *input = new_imput;
    return (free(str), code);
}

/**
 * Reads a line of input with continuation prompt
 * 
 * @param data  Shell context containing state and settings
 * 
 * @return The input line read or NULL if interrupted
 * 
 * Displays a continuation prompt ("> ") and reads from standard input.
 * Sets pipe_interupt flag if interrupted by a signal.
 */
static char	*read_prompt_input(t_shell *data)
{
    char	*str;

    write(STDOUT_FILENO, "> ", 2);
    str = get_next_line(STDIN_FILENO, 0);
    if (g_exit_status == 19)
    {
        data->pipe_interupt = 1;
        g_exit_status = 130;
    }
    return (str);
}

/**
 * Processes a line of command input
 * 
 * @param data   Shell context containing state and settings
 * @param input  Current command buffer (may be modified)
 * 
 * @return Status code indicating next action:
 *         0 = command complete
 *         1 = exit or error 
 *         2 = empty line, continue reading
 *         3 = line ends with pipe, continue reading
 * 
 * Handles reading input, stripping newlines, checking for empty lines,
 * and determining if the command continues due to a pipe.
 */
static int	process_command_line_input(t_shell *data, char **input)
{
    char	*str;

    if (*input && (*input)[0] == '|')
        return (0);
    str = read_prompt_input(data);
    if (data->pipe_interupt)
        return (free(str), free(*input), *input = NULL, 1);
    if (!str)
    {
        write(STDOUT_FILENO, "\nCommand canceled\n", 18);
        return (free(*input), *input = NULL, 1);
    }
    if (str[ft_strlen(str) - 1] == '\n')
        str[ft_strlen(str) - 1] = '\0';
    if (str[0] == '\0' || is_whitespace_bis(str))
        return (free(str), 2);
    if (end_with_pipe(str))
        return (append_to_command_input(input, str, 3));
    return (append_to_command_input(input, str, 0));
}

/**
 * Reads a complete command, handling multi-line inputs with pipes
 * 
 * @param data   Shell context containing state and settings
 * @param input  Pointer to store the complete command (modified)
 * 
 * @return Status code:
 *         0 = success (command ready)
 *         1 = error or interrupted
 * 
 * Configures signal handling for command input and continues reading
 * until a complete command is entered or interrupted.
 * Restores original signal handlers afterward.
 */
int	read_complete_command(t_shell *data, char **input)
{
    struct sigaction	old_int;
    struct sigaction	old_quit;
    int					result;

    data->pipe_interupt = 0;
    setup_heredoc_signal_handlers(&old_int, &old_quit);
    while (1)
    {
        result = process_command_line_input(data, input);
        if (data->pipe_interupt)
        {
            restore_signals_clear_buffer(&old_int, &old_quit);
            return (1);
        }
        if (result == 2 || result == 3)
            continue ;
        if (result != 0)
        {
            restore_signals_clear_buffer(&old_int, &old_quit);
            return (result);
        }
        break ;
    }
    restore_signals_clear_buffer(&old_int, &old_quit);
    return (0);
}
