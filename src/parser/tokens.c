/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokens.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:53 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/10 21:46:19 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

/**
 * Forward declaration to fix implicit declaration error
 */
static void	free_cmd(t_cmd *cmd);

/**
 * Create a new command structure
 */
t_cmd	*create_cmd(void)
{
    t_cmd	*cmd;

    cmd = (t_cmd *)malloc(sizeof(t_cmd));
    if (!cmd)
        return (NULL);
    cmd->args = NULL;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->heredoc_delim = NULL;
    cmd->heredoc_file = NULL;
    cmd->input_fd = -1;
    cmd->output_fd = -1;
    cmd->append_mode = 0;
    cmd->redirections = NULL;
    cmd->next = NULL;
    return (cmd);
}

/**
 * Initialize arguments array with the first argument
 */
int	init_args(t_cmd *cmd, char *arg)
{
    if (!cmd || !arg)
        return (0);
    cmd->args = (char **)malloc(sizeof(char *) * 2);
    if (!cmd->args)
    {
        free(arg);
        return (0);
    }
    cmd->args[0] = arg;
    cmd->args[1] = NULL;
    return (1);
}

/**
 * Copy existing arguments to a new array
 */
static char	**copy_args(t_cmd *cmd, int count)
{
    char	**new_args;
    int		i;

    new_args = (char **)malloc(sizeof(char *) * (count + 2));
    if (!new_args)
        return (NULL);
    i = 0;
    while (cmd->args[i])
    {
        new_args[i] = cmd->args[i];
        i++;
    }
    return (new_args);
}

/**
 * Add an argument to the command's args array
 */
void	add_arg(t_cmd *cmd, char *arg)
{
    int		i;
    char	**new_args;

    if (!cmd || !arg)
        return ;
    if (!cmd->args)
    {
        if (!init_args(cmd, arg))
            free(arg);
        return ;
    }
    i = 0;
    while (cmd->args[i])
        i++;
    new_args = copy_args(cmd, i);
    if (!new_args)
    {
        free(arg);
        return ;
    }
    new_args[i] = arg;
    new_args[i + 1] = NULL;
    free(cmd->args);
    cmd->args = new_args;
}

/**
 * Free a command list
 */
void	free_cmd_list(t_cmd *cmd)
{
    t_cmd	*current;
    t_cmd	*next;

    current = cmd;
    while (current)
    {
        next = current->next;
        free_cmd(current);
        current = next;
    }
}

/**
 * Free redirections in a command
 */
static void	free_redirections(t_redirection *redir)
{
    t_redirection	*next_redir;

    while (redir)
    {
        next_redir = redir->next;
        if (redir->word)
            free(redir->word);
        if (redir->temp_file)
            free(redir->temp_file);
        free(redir);
        redir = next_redir;
    }
}

/**
 * Free a single command and its resources
 */
static void	free_cmd(t_cmd *cmd)
{
    int	i;

    if (!cmd)
        return ;
    if (cmd->args)
    {
        i = 0;
        while (cmd->args[i])
            free(cmd->args[i++]);
        free(cmd->args);
    }
    if (cmd->input_file)
        free(cmd->input_file);
    if (cmd->output_file)
        free(cmd->output_file);
    if (cmd->heredoc_delim)
        free(cmd->heredoc_delim);
    if (cmd->heredoc_file)
        free(cmd->heredoc_file);
    free_redirections(cmd->redirections);
    if (cmd->input_fd >= 0)
        close(cmd->input_fd);
    if (cmd->output_fd >= 0)
        close(cmd->output_fd);
    free(cmd);
}

/**
 * Join consecutive tokens without spaces
 */
static char	*join_consecutive_tokens(char *word, t_token **current)
{
    t_token	*next;
    char	*temp;

    next = (*current)->next;
    while (next && (next->type == TOKEN_WORD || 
            next->type == TOKEN_SINGLE_QUOTE ||
            next->type == TOKEN_DOUBLE_QUOTE))
    {
        if (next->preceded_by_space)
            break ;
        temp = word;
        word = ft_strjoin(word, next->value);
        free(temp);
        if (!word)
            return (NULL);
        *current = next;
        next = next->next;
    }
    return (word);
}

/**
 * Process word tokens and handle token joining when no spaces
 */
void	join_word_tokens(t_cmd *cmd, t_token **token)
{
    char	*word;
    t_token	*current;
    int		is_first_arg;

    is_first_arg = (cmd->args == NULL);
    word = ft_strdup((*token)->value);
    if (!word)
        return ;
    current = *token;
    if (is_first_arg)
    {
        word = join_consecutive_tokens(word, &current);
        if (!word)
            return ;
    }
    add_arg(cmd, word);
    *token = current;
}

/**
 * Check if a token is a valid argument token
 */
int	is_arg_token(t_token *token)
{
    if (!token)
        return (0);
    return (token->type == TOKEN_WORD ||
            token->type == TOKEN_SINGLE_QUOTE ||
            token->type == TOKEN_DOUBLE_QUOTE);
}
