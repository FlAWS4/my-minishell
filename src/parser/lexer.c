/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:10 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/11 21:40:32 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Checks if a character is a special token character
 */
int	is_special(char c)
{
    return (c == '|' || c == '<' || c == '>');
}

/**
 * Create a new token
 */
t_token	*create_token(t_token_type type, char *value, int preceded_by_space)
{
    t_token	*new_token;

    new_token = (t_token *)malloc(sizeof(t_token));
    if (!new_token)
    {
        free(value);
        return (NULL);
    }
    new_token->type = type;
    new_token->value = value;
    new_token->preceded_by_space = preceded_by_space;
    new_token->next = NULL;
    return (new_token);
}

/**
 * Add a token to the token list
 */
void	add_token(t_token **list, t_token *new)
{
    t_token	*temp;

    if (!new)
        return ;
    if (!*list)
    {
        *list = new;
        return ;
    }
    temp = *list;
    while (temp->next)
        temp = temp->next;
    temp->next = new;
}

/**
 * Get the last token in a list
 */
t_token	*get_last_token(t_token *tokens)
{
    t_token	*last;

    if (!tokens)
        return (NULL);
    last = tokens;
    while (last->next)
        last = last->next;
    return (last);
}

/**
 * Free the entire token list
 */
void	free_token_list(t_token *tokens)
{
    t_token	*temp;

    while (tokens)
    {
        temp = tokens;
        tokens = tokens->next;
        free(temp->value);
        free(temp);
    }
}

/**
 * Handle pipe token
 */
static int	create_pipe_token(t_token **tokens, int preceded_by_space)
{
    char	*value;
    t_token	*token;

    if (!tokens)
        return (-1);
    value = ft_strdup("|");
    if (!value)
        return (-1);
    token = create_token(TOKEN_PIPE, value, preceded_by_space);
    if (!token)
    {
        free(value);
        return (-1);
    }
    add_token(tokens, token);
    return (0);
}

/**
 * Create heredoc token
 */
static int	create_heredoc_token(t_token **tokens, int preceded_by_space)
{
    char	*value;
    t_token	*token;

    value = ft_strdup("<<");
    if (!value)
        return (-1);
    token = create_token(TOKEN_HEREDOC, value, preceded_by_space);
    if (!token)
    {
        free(value);
        return (-1);
    }
    add_token(tokens, token);
    return (0);
}

/**
 * Create input redirection token
 */
static int	create_input_token(t_token **tokens, int preceded_by_space)
{
    char	*value;
    t_token	*token;

    value = ft_strdup("<");
    if (!value)
        return (-1);
    token = create_token(TOKEN_REDIR_IN, value, preceded_by_space);
    if (!token)
    {
        free(value);
        return (-1);
    }
    add_token(tokens, token);
    return (0);
}

/**
 * Handle input redirection tokens
 */
static int	handle_input_redir(char *input, int i, t_token **tokens,
        int preceded_by_space)
{
    if (!input || !tokens)
        return (-1);
    if (input[i + 1] && input[i + 1] == '<')
    {
        if (create_heredoc_token(tokens, preceded_by_space) == -1)
            return (-1);
        return (i + 2);
    }
    if (create_input_token(tokens, preceded_by_space) == -1)
        return (-1);
    return (i + 1);
}

/**
 * Create append redirection token
 */
static int	create_append_token(t_token **tokens, int preceded_by_space)
{
    char	*value;
    t_token	*token;

    value = ft_strdup(">>");
    if (!value)
        return (-1);
    token = create_token(TOKEN_REDIR_APPEND, value, preceded_by_space);
    if (!token)
    {
        free(value);
        return (-1);
    }
    add_token(tokens, token);
    return (0);
}

/**
 * Create output redirection token
 */
static int	create_output_token(t_token **tokens, int preceded_by_space)
{
    char	*value;
    t_token	*token;

    value = ft_strdup(">");
    if (!value)
        return (-1);
    token = create_token(TOKEN_REDIR_OUT, value, preceded_by_space);
    if (!token)
    {
        free(value);
        return (-1);
    }
    add_token(tokens, token);
    return (0);
}

/**
 * Handle output redirection tokens
 */
static int	handle_output_redir(char *input, int i, t_token **tokens,
        int preceded_by_space)
{
    if (!input || !tokens)
        return (-1);
    if (input[i + 1] && input[i + 1] == '>')
    {
        if (create_append_token(tokens, preceded_by_space) == -1)
            return (-1);
        return (i + 2);
    }
    if (create_output_token(tokens, preceded_by_space) == -1)
        return (-1);
    return (i + 1);
}

/**
 * Check if preceded by whitespace
 */
static int	check_preceded_by_space(char *input, int i)
{
    if (i > 0 && is_whitespace(input[i - 1]))
        return (1);
    return (0);
}

/**
 * Handle pipe character
 */
static int	handle_pipe_char(char *input, int i, t_token **tokens)
{
    int	preceded_by_space;

    preceded_by_space = check_preceded_by_space(input, i);
    if (create_pipe_token(tokens, preceded_by_space) == -1)
        return (-1);
    return (i + 1);
}

/**
 * Handle special characters (pipes, redirections)
 */
int	handle_special(char *input, int i, t_token **tokens)
{
    int	preceded_by_space;
    int	result;

    if (!input || !tokens || i < 0)
        return (-1);
    preceded_by_space = check_preceded_by_space(input, i);
    if (input[i] == '|')
        return (handle_pipe_char(input, i, tokens));
    else if (input[i] == '<')
        result = handle_input_redir(input, i, tokens, preceded_by_space);
    else if (input[i] == '>')
        result = handle_output_redir(input, i, tokens, preceded_by_space);
    else
        result = i + 1;
    return (result);
}

/**
 * Create quote token
 */
static int	create_quote_token(char *word, char quote, int preceded_by_space,
        t_token **tokens)
{
    t_token	*token;
    t_token_type	type;

    if (quote == '\'')
        type = TOKEN_SINGLE_QUOTE;
    else
        type = TOKEN_DOUBLE_QUOTE;
    token = create_token(type, word, preceded_by_space);
    if (!token)
        return (-1);
    add_token(tokens, token);
    return (0);
}

/**
 * Handle quoted strings in input
 */
int	handle_quote(char *input, int i, t_token **tokens)
{
    char	quote;
    int		start;
    char	*word;
    int		preceded_by_space;

    preceded_by_space = check_preceded_by_space(input, i);
    quote = input[i];
    start = i + 1;
    i++;
    while (input[i] && input[i] != quote)
        i++;
    if (!input[i])
        return (-1);
    word = ft_substr(input, start, i - start);
    if (!word)
        return (-1);
    if (create_quote_token(word, quote, preceded_by_space, tokens) == -1)
    {
        free(word);
        return (-1);
    }
    return (i + 1);
}

/**
 * Extract word boundaries in input
 */
static int	find_word_end(char *input, int i)
{
    int	start;

    start = i;
    while (input[i] && !is_whitespace(input[i]) && !is_special(input[i])
        && input[i] != '\'' && input[i] != '\"')
    {
        // More explicit check to prevent reading past end of string
        if (input[i] == '\\' && input[i + 1] != '\0')
            i++;
        i++;
    }
    return (i);
}

/**
 * Process escape sequences in word
 */
static char	*process_escape_sequences(char *word, char *input, int start)
{
    int		i;
    int		j;
    char	*processed_word;

    processed_word = malloc(ft_strlen(word) + 1);
    if (!processed_word)
    {
        free(word);
        return (NULL);
    }
    i = start;
    j = 0;
    while (input[i] && i < start + (int)ft_strlen(word))
    {
        if (input[i] == '\\' && input[i + 1])
            processed_word[j++] = input[++i];
        else
            processed_word[j++] = input[i];
        i++;
    }
    processed_word[j] = '\0';
    return (processed_word);
}

/**
 * Create word token
 */
static int	create_word_token(char *word, int preceded_by_space,
        t_token **tokens)
{
    t_token	*token;

    token = create_token(TOKEN_WORD, word, preceded_by_space);
    if (!token)
    {
        free(word);
        return (-1);
    }
    add_token(tokens, token);
    return (0);
}

/**
 * Handle word token in input with escape character support
 */
int	handle_word(char *input, int i, t_token **tokens)
{
    int		start;
    char	*word;
    int		preceded_by_space;
    char	*processed_word;

    preceded_by_space = check_preceded_by_space(input, i);
    start = i;
    i = find_word_end(input, i);
    word = ft_substr(input, start, i - start);
    if (!word)
        return (-1);
    processed_word = process_escape_sequences(word, input, start);
    if (!processed_word)
        return (-1);
    if (create_word_token(processed_word, preceded_by_space, tokens) == -1)
        return (-1);
    free(word);
    return (i);
}

/**
 * Validate pipe token syntax
 */
static int	validate_pipe_syntax(t_token *current, t_token *next, t_token *tokens)
{
    if (!next || current == tokens)
    {
        print_syntax_error("|", TOKEN_PIPE);
        return (0);
    }
    if (next->type == TOKEN_PIPE)
    {
        print_syntax_error("|", TOKEN_PIPE);
        return (0);
    }
    return (1);
}

/**
 * Check if redirection has valid target
 */
static int	has_valid_redir_target(t_token *current, t_token *next)
{
    (void)current; /* Mark as unused to prevent compiler warning */
    if (!next)
        return (0);
    if (next->type == TOKEN_PIPE || is_redirection_token(next))
        return (0);
    return (1);
}

/**
 * Validate redirection token syntax
 */
static int	validate_redirection_syntax(t_token *current, t_token *next)
{
    if (!has_valid_redir_target(current, next))
    {
        if (!next)
            print_syntax_error("newline", -1);
        else
            print_syntax_error(current->value, current->type);
        return (0);
    }
    return (1);
}

/**
 * Validate token syntax with improved error checking
 */
int	validate_syntax(t_token *tokens)
{
    t_token	*current;
    t_token	*next;

    if (!tokens)
        return (1);
    current = tokens;
    while (current)
    {
        next = current->next;
        if (current->type == TOKEN_PIPE)
        {
            if (!validate_pipe_syntax(current, next, tokens))
                return (0);
        }
        if (is_redirection_token(current))
        {
            if (!validate_redirection_syntax(current, next))
                return (0);
        }
        current = next;
    }
    return (1);
}

/**
 * Check for unclosed quotes in input string
 */
static int	handle_quote_char(char c, int *in_single_quote,
        int *in_double_quote)
{
    if (c == '\'' && !(*in_double_quote))
        *in_single_quote = !(*in_single_quote);
    else if (c == '\"' && !(*in_single_quote))
        *in_double_quote = !(*in_double_quote);
    return (0);
}

/**
 * Check for unclosed quotes in input string
 */
int	check_unclosed_quotes(char *input)
{
    int	i;
    int	in_single_quote;
    int	in_double_quote;

    if (!input)
        return (1);
    i = 0;
    in_single_quote = 0;
    in_double_quote = 0;
    while (input[i])
    {
        handle_quote_char(input[i], &in_single_quote, &in_double_quote);
        i++;
    }
    if (in_single_quote || in_double_quote)
    {
        ft_putstr_fd("minishell: syntax error: unclosed quotes\n",
            STDERR_FILENO);
        return (1);
    }
    return (0);
}

/**
 * Determine if tokens should be merged
 */
static int	should_merge_tokens(t_token *current, t_token *next)
{
    if (!next->preceded_by_space && 
        ((current->type == TOKEN_WORD || 
        current->type == TOKEN_SINGLE_QUOTE || 
        current->type == TOKEN_DOUBLE_QUOTE) && 
        (next->type == TOKEN_WORD ||
        next->type == TOKEN_SINGLE_QUOTE ||
        next->type == TOKEN_DOUBLE_QUOTE)))
        return (1);
    return (0);
}

/**
 * Merge two adjacent tokens
 */
static int	merge_tokens(t_token *current, t_token *next)
{
    char	*merged_value;

    merged_value = ft_strjoin(current->value, next->value);
    if (!merged_value)
        return (0);
    free(current->value);
    current->value = merged_value;
    current->type = TOKEN_WORD;
    return (1);
}

/**
 * Remove token from list
 */
static void	remove_token(t_token *current, t_token *to_remove)
{
    current->next = to_remove->next;
    free(to_remove->value);
    free(to_remove);
}

/**
 * Merge adjacent quoted tokens and words
 */
void	merge_adjacent_quoted_tokens(t_token **tokens)
{
    t_token	*current;
    t_token	*next;
    t_token	*temp;

    if (!tokens || !*tokens)
        return ;
    current = *tokens;
    while (current && current->next)
    {
        next = current->next;
        if (should_merge_tokens(current, next))
        {
            if (!merge_tokens(current, next))
                return ;
            temp = next;
            remove_token(current, temp);
        }
        else
            current = current->next;
    }
}

/**
 * Handle whitespace in token processing
 */
static int	handle_whitespace(int i, int *was_space)
{
    *was_space = 1;
    return (i + 1);
}

/**
 * Update token space flag
 */
static void	update_token_space(t_token **tokens, int *was_space)
{
    t_token	*last;

    last = get_last_token(*tokens);
    if (last)
        last->preceded_by_space = *was_space;
    *was_space = 0;
}

/**
 * Process a token at the current position
 */
static int	process_token_part(char *input, int i, t_token **tokens,
        int *was_space)
{
    if (is_whitespace(input[i]))
        return (handle_whitespace(i, was_space));
    else if (input[i] == '\'' || input[i] == '\"')
    {
        i = handle_quote(input, i, tokens);
        if (i == -1)
            return (-1);
    }
    else if (is_special(input[i]))
    {
        i = handle_special(input, i, tokens);
        if (i == -1)
            return (-1);
    }
    else
    {
        i = handle_word(input, i, tokens);
        if (i == -1)
            return (-1);
    }
    update_token_space(tokens, was_space);
    return (i);
}

/**
 * Handle token processing error
 */
static t_token	*handle_token_error(t_token *tokens)
{
    free_token_list(tokens);
    ft_putstr_fd("minishell: syntax error: unclosed quote\n", 2);
    return (NULL);
}

/**
 * Process tokens from input string
 */
t_token	*process_tokens(char *input)
{
    t_token	*tokens;
    int		i;
    int		was_space;

    tokens = NULL;
    i = 0;
    was_space = 1;
    while (input[i])
    {
        i = process_token_part(input, i, &tokens, &was_space);
        if (i == -1)
            return (handle_token_error(tokens));
    }
    return (tokens);
}

/**
 * Tokenize the input string
 */
t_token	*tokenize(char *input)
{
    t_token	*tokens;

    if (!input)
        return (NULL);
    if (check_unclosed_quotes(input))
        return (NULL);
    tokens = process_tokens(input);
    if (!tokens)
        return (NULL);
    merge_adjacent_quoted_tokens(&tokens);
    if (!validate_syntax(tokens))
    {
        free_token_list(tokens);
        return (NULL);
    }
    return (tokens);
}

/**
 * Process input string into tokens, performing variable expansion
 */
t_token	*tokenize_and_expand(char *input, t_shell *shell)
{
    t_token	*tokens;

    if (!input || !*input)
        return (NULL);
    if (*input)
        add_history(input);
    tokens = tokenize(input);
    if (!tokens)
    {
        shell->exit_status = 2;
        return (NULL);
    }
    tokens = expand_variables_in_tokens_with_splitting(tokens, shell);
    return (tokens);
}
