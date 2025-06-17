/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokens.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:53 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 01:52:07 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

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
    cmd->heredocs_processed = 0;
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
    new_args[i] = NULL;
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
void	free_cmd(t_cmd *cmd)
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
    {
        unlink(cmd->heredoc_file);
        free(cmd->heredoc_file);
    }
    free_redirections(cmd->redirections);
    if (cmd->input_fd >= 2)
        close(cmd->input_fd);
    if (cmd->output_fd >= 2)
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
    char	*result;

    next = (*current)->next;
    while (next && (next->type == TOKEN_WORD || 
            next->type == TOKEN_SINGLE_QUOTE ||
            next->type == TOKEN_DOUBLE_QUOTE))
    {
        if (next->preceded_by_space)
            break;
        temp = word;
        result = ft_strjoin(word, next->value);
        if (!result)
        {
            free(temp);
            return (NULL);
        }
        free(temp);
        word = result;
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

    word = ft_strdup((*token)->value);
    if (!word)
        return ;
    current = *token;
    word = join_consecutive_tokens(word, &current);
    if (!word)
        return ;
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

/**
 * Handle quoted argument processing
 */
static int	handle_quote_prefix(char **arg, int *is_quoted)
{
    char	*temp;
    
    if ((*arg)[0] == '\'' || (*arg)[0] == '"')
    {
        if ((*arg)[1] == ':')
        {
            *is_quoted = 1;
            temp = ft_strdup(*arg + 2);
            if (!temp)
                return (0);
            free(*arg);
            *arg = temp;
            return (1);
        }
    }
    else if ((*arg)[0] == ':')
    {
        *is_quoted = 1;
        temp = ft_strdup(*arg + 1);
        if (!temp)
            return (0);
        free(*arg);
        *arg = temp;
        return (1);
    }
    return (1);
}

/**
 * Process and expand a single argument
 */
static int	process_arg(t_cmd *cmd, int i, t_shell *shell, int *reset)
{
    char	*original;
    int		is_quoted;
    int		is_standalone;
    char	*expanded;

    original = ft_strdup(cmd->args[i]);
    if (!original)
        return (1);
    is_quoted = 0;
    if (!handle_quote_prefix(&cmd->args[i], &is_quoted))
    {
        free(original);
        return (1);
    }
    is_standalone = (original[0] == '$' && 
                  !ft_strchr(original, '\'') && 
                  !ft_strchr(original, '\"'));
    free(original);
    expanded = expand_variables(shell, cmd->args[i]);
    if (!expanded)
        return (1);
    free(cmd->args[i]);
    cmd->args[i] = expanded;
    *reset = (i == 0 && is_standalone && !is_quoted && 
        expanded[0] && ft_strchr(expanded, ' '));
    return (0);
}

/**
 * Create a new arguments array from split result
 */
static char	**create_new_args(char **split, int count)
{
    char	**new_args;
    int		j;

    new_args = malloc(sizeof(char *) * (count + 1));
    if (!new_args)
        return (NULL);
    j = 0;
    while (j < count)
    {
        new_args[j] = ft_strdup(split[j]);
        if (!new_args[j])
        {
            while (--j >= 0)
                free(new_args[j]);
            free(new_args);
            return (NULL);
        }
        j++;
    }
    new_args[count] = NULL;
    return (new_args);
}

/**
 * Handle splitting of first argument if needed
 */
static int	handle_arg_splitting(t_cmd *cmd, int *should_reset)
{
    char	**split;
    char	**new_args;
    int		count;

    *should_reset = 0;
    split = ft_split(cmd->args[0], ' ');
    if (!split || !split[0])
    {
        if (split)
            free_env_array(split);
        return (0);
    }
    count = 0;
    while (split[count])
        count++;
    new_args = create_new_args(split, count);
    if (!new_args)
    {
        free_env_array(split);
        return (1);
    }
    free(cmd->args[0]);
    free(cmd->args);
    cmd->args = new_args;
    free_env_array(split);
    *should_reset = 1;
    return (0);
}

/**
 * Expand command arguments and handle variable splitting
 */
int	expand_command_args(t_cmd *cmd_list, t_shell *shell)
{
    t_cmd	*current;
    int		i;
    int		should_reset;
    int		error;

    if (!cmd_list || !shell)
        return (1);
    current = cmd_list;
    while (current)
    {
        if (current->args)
        {
            i = 0;
            while (current->args[i])
            {
                error = process_arg(current, i, shell, &should_reset);
                if (error)
                    i++;
                else if (should_reset && handle_arg_splitting(current, &should_reset) == 0
                    && should_reset)
                    i = 0;
                else
                    i++;
            }
        }
        current = current->next;
    }
    return (0);
}
