/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:10 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 22:25:16 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Checks if a character is a special token character
 */
int is_special(char c)
{
    return (c == '|' || c == '<' || c == '>');
}

/**
 * Create a new token
 */
t_token *create_token(t_token_type type, char *value, int preceded_by_space)
{
    t_token *new_token;

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
void add_token(t_token **list, t_token *new)
{
    t_token *temp;

    if (!new)
        return;
    if (!*list)
    {
        *list = new;
        return;
    }
    temp = *list;
    while (temp->next)
        temp = temp->next;
    temp->next = new;
}

/**
 * Get the last token in a list
 */
t_token *get_last_token(t_token *tokens)
{
    t_token *last;

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
void free_token_list(t_token *tokens)
{
    t_token *temp;

    while (tokens)
    {
        temp = tokens;
        tokens = tokens->next;
        free(temp->value);
        free(temp);
    }
}

/**
 * Create and add a token with given type and value
 */
static int create_and_add_token(t_token **tokens, t_token_type type, 
                              char *value, int preceded_by_space)
{
    t_token *token;
    
    if (!value)
        return (-1);
        
    token = create_token(type, value, preceded_by_space);
    if (!token)
        return (-1);
        
    add_token(tokens, token);
    return (0);
}

/**
 * Check if preceded by whitespace
 */
static int check_preceded_by_space(char *input, int i)
{
    if (i > 0 && is_whitespace(input[i - 1]))
        return (1);
    return (0);
}

/**
 * Handle input redirection tokens
 */
static int handle_input_redir(char *input, int i, t_token **tokens)
{
    int preceded_by_space = check_preceded_by_space(input, i);
    char *token_value;
    
    if (input[i + 1] && input[i + 1] == '<')
    {
        token_value = ft_strdup("<<");
        if (!token_value)
            return (-1);
        if (create_and_add_token(tokens, TOKEN_HEREDOC, token_value, preceded_by_space) == -1)
        {
            free(token_value);
            return (-1);
        }
        return (i + 2);
    }
    
    token_value = ft_strdup("<");
    if (!token_value)
        return (-1);
    if (create_and_add_token(tokens, TOKEN_REDIR_IN, token_value, preceded_by_space) == -1)
    {
        free(token_value);
        return (-1);
    }
    return (i + 1);
}

/**
 * Handle output redirection tokens
 */
static int handle_output_redir(char *input, int i, t_token **tokens)
{
    int preceded_by_space = check_preceded_by_space(input, i);
    char *token_value;
    
    if (input[i + 1] && input[i + 1] == '>')
    {
        token_value = ft_strdup(">>");
        if (!token_value)
            return (-1);
        if (create_and_add_token(tokens, TOKEN_REDIR_APPEND, token_value, preceded_by_space) == -1)
        {
            free(token_value);
            return (-1);
        }
        return (i + 2);
    }
    
    token_value = ft_strdup(">");
    if (!token_value)
        return (-1);
    if (create_and_add_token(tokens, TOKEN_REDIR_OUT, token_value, preceded_by_space) == -1)
    {
        free(token_value);
        return (-1);
    }
    return (i + 1);
}

/**
 * Handle special characters (pipes, redirections)
 */
int handle_special(char *input, int i, t_token **tokens)
{
    int preceded_by_space;
    char *token_value;
    
    if (!input || !tokens)
        return (-1);
        
    preceded_by_space = check_preceded_by_space(input, i);
    if (input[i] == '|')
    {
        token_value = ft_strdup("|");
        if (!token_value)
            return (-1);
        if (create_and_add_token(tokens, TOKEN_PIPE, token_value, preceded_by_space) == -1)
        {
            free(token_value);
            return (-1);
        }
        return (i + 1);
    }
    else if (input[i] == '<')
        return (handle_input_redir(input, i, tokens));
    else if (input[i] == '>')
        return (handle_output_redir(input, i, tokens));
    
    return (i + 1);
}

/**
 * Handle quoted strings in input
 */
int handle_quote(char *input, int i, t_token **tokens)
{
    char quote;
    int start;
    char *word;
    int preceded_by_space;
    t_token_type type;

    preceded_by_space = check_preceded_by_space(input, i);
    quote = input[i];
    type = (quote == '\'') ? TOKEN_SINGLE_QUOTE : TOKEN_DOUBLE_QUOTE;
    
    start = i + 1;
    i++;
    while (input[i] && input[i] != quote)
        i++;
    if (!input[i])
        return (-1);
    
    word = ft_substr(input, start, i - start);
    if (!word)
        return (-1);
        
    if (create_and_add_token(tokens, type, word, preceded_by_space) == -1)
    {
        free(word);
        return (-1);
    }
    
    return (i + 1);
}

/**
 * Extract word boundaries in input
 */
static int find_word_end(char *input, int i)
{
    while (input[i] && !is_whitespace(input[i]) && !is_special(input[i])
        && input[i] != '\'' && input[i] != '\"')
    {
        if (input[i] == '\\' && input[i + 1])
            i++;
        i++;
    }
    return (i);
}

/**
 * Process escape sequences in word
 */
static char *process_escape_sequences(char *word, char *input, int start)
{
    int i, j;
    char *processed_word;

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
 * Handle word token in input with escape character support
 */
int handle_word(char *input, int i, t_token **tokens)
{
    int start;
    char *word, *processed_word;
    int preceded_by_space;

    preceded_by_space = check_preceded_by_space(input, i);
    start = i;
    i = find_word_end(input, i);
    
    word = ft_substr(input, start, i - start);
    if (!word)
        return (-1);
        
    processed_word = process_escape_sequences(word, input, start);
    if (!processed_word)
        return (-1);
    if (create_and_add_token(tokens, TOKEN_WORD, processed_word, preceded_by_space) == -1)
    {
        free(processed_word);
        free(word);
        return (-1);
    }
        
    free(word);
    return (i);
}

/**
 * Check if redirection has valid target
 */
int is_valid_redir_target(t_token *next)
{
    if (!next)
        return (0);
    if (next->type == TOKEN_PIPE || is_redirection_token(next))
        return (0);
    return (1);
}

/**
 * Validate token syntax with improved error checking
 */
int validate_syntax(t_token *tokens)
{
    t_token *current, *next;

    if (!tokens)
        return (1);
        
    current = tokens;
    while (current)
    {
        next = current->next;
        
        if (current->type == TOKEN_PIPE)
        {
            // Validate pipe syntax
            if (!next || current == tokens || next->type == TOKEN_PIPE)
            {
                print_syntax_error("|", TOKEN_PIPE);
                return (0);
            }
        }
        else if (is_redirection_token(current))
        {
            // Validate redirection syntax
            if (!is_valid_redir_target(next))
            {
                if (!next)
                    print_syntax_error("newline", -1);
                else
                    print_syntax_error(current->value, current->type);
                return (0);
            }
        }
        
        current = next;
    }
    return (1);
}

/**
 * Check for unclosed quotes in input string
 */
static int check_unclosed_quotes(char *input)
{
    int i, in_single_quote = 0, in_double_quote = 0;

    if (!input)
        return (1);
        
    for (i = 0; input[i]; i++)
    {
        if (input[i] == '\'' && !in_double_quote)
            in_single_quote = !in_single_quote;
        else if (input[i] == '\"' && !in_single_quote)
            in_double_quote = !in_double_quote;
    }
    
    if (in_single_quote || in_double_quote)
    {
        ft_putstr_fd("minishell: syntax error: unclosed quotes\n", STDERR_FILENO);
        return (1);
    }
    return (0);
}

/**
 * Merge two adjacent tokens
 */
static int merge_tokens(t_token *current, t_token *next)
{
    char *merged_value;

    if (!current || !next)
        return (0);
        
    merged_value = ft_strjoin(current->value, next->value);
    if (!merged_value)
        return (0);  // Caller handles cleanup
    free(current->value);
    current->value = merged_value;
    current->type = TOKEN_WORD;
    return (1);
}

/**
 * Merge adjacent quoted tokens and words
 */
void merge_adjacent_quoted_tokens(t_token **tokens)
{
    t_token *current, *next, *temp;

    if (!tokens || !*tokens)
        return;
        
    current = *tokens;
    while (current && current->next)
    {
        next = current->next;
        
        if (!next->preceded_by_space && 
        ((current->type == TOKEN_WORD || 
          current->type == TOKEN_SINGLE_QUOTE || 
          current->type == TOKEN_DOUBLE_QUOTE) && 
         (next->type == TOKEN_WORD ||
          next->type == TOKEN_SINGLE_QUOTE ||
          next->type == TOKEN_DOUBLE_QUOTE)))
        {
            if (!merge_tokens(current, next))
            {
            // Cannot merge - just continue to next token
                current = current->next;
                continue;
            }
            temp = next;
            current->next = temp->next;
            free(temp->value);
            free(temp);
        }
        else
            current = current->next;
    }
}

/**
 * Process tokens from input string
 */
t_token *process_tokens(char *input)
{
    t_token *tokens = NULL;
    int i = 0, was_space = 1;
    t_token *last;

    while (input[i])
    {
        if (is_whitespace(input[i]))
        {
            was_space = 1;
            i++;
        }
        else if (input[i] == '\'' || input[i] == '\"')
        {
            i = handle_quote(input, i, &tokens);
            if (i == -1)
            {
                free_token_list(tokens);
                display_error(ERROR_SYNTAX, NULL, "unclosed quote");
                return (NULL);
            }
            
            // Update space flag for last token
            last = get_last_token(tokens);
            if (last)
                last->preceded_by_space = was_space;
            was_space = 0;
        }
        else if (is_special(input[i]))
        {
            i = handle_special(input, i, &tokens);
            if (i == -1)
            {
                free_token_list(tokens);
                return (NULL);
            }
            
            // Update space flag for last token
            last = get_last_token(tokens);
            if (last)
                last->preceded_by_space = was_space;
            was_space = 0;
        }
        else
        {
            i = handle_word(input, i, &tokens);
            if (i == -1)
            {
                free_token_list(tokens);
                return (NULL);
            }
            
            // Update space flag for last token
            last = get_last_token(tokens);
            if (last)
                last->preceded_by_space = was_space;
            was_space = 0;
        }
    }
    
    return (tokens);
}

/**
 * Tokenize the input string
 */
t_token *tokenize(char *input)
{
    t_token *tokens;

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
t_token *tokenize_and_expand(char *input, t_shell *shell)
{
    t_token *tokens;
    t_token *current;
    char *expanded_value;
    int skip_next;

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
    
    current = tokens;
    skip_next = 0;
    while (current)
    {
        // Handle heredoc delimiters with proper quote detection
        if (current->type == TOKEN_HEREDOC)
        {
            skip_next = 1;
            current = current->next;
            continue;
        }
        
        if (skip_next)
        {
            // This is a heredoc delimiter - don't expand variables in it
            // But do mark it as quoted if it has single quotes
            if (current->type == TOKEN_SINGLE_QUOTE)
            {
                // Mark this token so parser knows to disable var expansion in heredoc
                current->preceded_by_space = 2; // Special flag for single-quoted heredoc delimiters
            }
            skip_next = 0;
            current = current->next;
            continue;
        }
        
        // Normal token expansion for non-heredoc tokens
        if (current && (current->type == TOKEN_WORD || current->type == TOKEN_DOUBLE_QUOTE))
        {
            expanded_value = expand_variables(shell, current->value);
            if (expanded_value)
            {
                free(current->value);
                current->value = expanded_value;
            }
        }
        
        if (current)
            current = current->next;
    }
    
    return (tokens);
}
