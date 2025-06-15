/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokens.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:53 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/15 04:52:01 by mshariar         ###   ########.fr       */
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
static char *join_consecutive_tokens(char *word, t_token **current)
{
    t_token *next;
    char    *temp;
    char    *result;

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
            free(temp);  // Free the original word if join fails
            return (NULL);
        }
        word = result;
        *current = next;
        next = next->next;
    }
    return (word);
}

/**
 * Process word tokens and handle token joining when no spaces
 */
void join_word_tokens(t_cmd *cmd, t_token **token)
{
    char    *word;
    t_token *current;

    // Removed the unnecessary is_first_arg check
    word = ft_strdup((*token)->value);
    if (!word)
        return;
    current = *token;
    
    // Always join consecutive tokens regardless of position
    word = join_consecutive_tokens(word, &current);
    if (!word)
        return;
        
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
 * Expand command arguments and handle variable splitting
 */
void expand_command_args(t_cmd *cmd_list, t_shell *shell)
{
    t_cmd *current = cmd_list;
    
    while (current)
    {
        if (current->args)
        {
            int i = 0;
            while (current->args[i])
            {
                // Save original value for detection logic
                char *original = ft_strdup(current->args[i]);
                if (!original)
                {
                    i++; // Avoid infinite loop if allocation fails
                    continue;
                }
                    
                // Check for quote markers
                int is_quoted = 0;
                char *temp = NULL;
                
                // Process quote prefixes (single quotes or double quotes)
                if ((original[0] == '\'' || original[0] == '"') && original[1] == ':')
                {
                    is_quoted = 1;
                    // Remove the quote marker before expansion
                    temp = ft_strdup(original + 2);
                    if (temp)
                    {
                        free(current->args[i]);
                        current->args[i] = temp;
                    }
                }
                else if (original[0] == ':')
                {
                    // Handle standalone colon prefix (from quoted variables)
                    is_quoted = 1;
                    temp = ft_strdup(original + 1);
                    if (temp)
                    {
                        free(current->args[i]);
                        current->args[i] = temp;
                    }
                }
                
                // Identify standalone variables (for word splitting)
                int is_standalone_var = (original[0] == '$' && 
                                      !ft_strchr(original, '\'') && 
                                      !ft_strchr(original, '\"'));
                
                // Free the original copy now that we've used it
                free(original);
                
                // Expand variables in the argument
                char *expanded = expand_variables(shell, current->args[i]);
                if (!expanded)
                {
                    i++; // Avoid infinite loop if expansion fails
                    continue;
                }
                    
                free(current->args[i]);
                current->args[i] = expanded;
                
                // Only perform word splitting on unquoted standalone variables in command position
                if (i == 0 && is_standalone_var && !is_quoted && 
                    expanded[0] != '\0' && ft_strchr(expanded, ' '))
                {
                    char **split = ft_split(expanded, ' ');
                    if (!split)
                    {
                        i++; // Avoid infinite loop if split fails
                        continue;
                    }
                    
                    if (split[0])
                    {
                        // Count the number of split parts
                        int split_count = 0;
                        while (split[split_count])
                            split_count++;
                            
                        // Create new args array
                        char **new_args = malloc(sizeof(char *) * (split_count + 1));
                        if (!new_args)
                        {
                            free_env_array(split);
                            i++; // Avoid infinite loop
                            continue;
                        }
                        
                        // Copy split parts
                        int j;
                        for (j = 0; j < split_count; j++)
                            new_args[j] = ft_strdup(split[j]);
                        new_args[split_count] = NULL;
                        
                        // Free the old args array
                        free(current->args[0]);
                        free(current->args);
                        
                        // Set the new args array
                        current->args = new_args;
                        
                        // Reset index to process the new array from the beginning
                        i = -1;
                    }
                    free_env_array(split);
                }
                i++;
            }
        }
        current = current->next;
    }
}
