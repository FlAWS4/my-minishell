/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:10 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/09 20:40:02 by mshariar         ###   ########.fr       */
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
 * Returns NULL if the list is empty
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
    t_token *token;

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
 * Handle input redirection tokens
 */
static int	handle_input_redir(char *input, int i, t_token **tokens, 
    int preceded_by_space)
{
    char	*value;
    t_token *token;

    if (!input || !tokens)
        return (-1);
        
    if (input[i + 1] && input[i + 1] == '<')
    {
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
        return (i + 2);  // Skip both '<' characters
    }
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
    return (i + 1);  // Skip the '<' character
}

/**
 * Handle output redirection tokens
 */
static int	handle_output_redir(char *input, int i, t_token **tokens, 
    int preceded_by_space)
{
    char	*value;
    t_token *token;

    if (!input || !tokens)
        return (-1);
        
    if (input[i + 1] && input[i + 1] == '>')
    {
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
        return (i + 2);  // Skip both '>' characters
    }
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
    return (i + 1);  // Skip the '>' character
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
        
    preceded_by_space = 0;
    if (i > 0 && is_whitespace(input[i - 1]))
        preceded_by_space = 1;
    
    if (input[i] == '|')
    {
        if (create_pipe_token(tokens, preceded_by_space) == -1)
            return (-1);
        return (i + 1);  // Return index of next character
    }
    else if (input[i] == '<')
        result = handle_input_redir(input, i, tokens, preceded_by_space);
    else if (input[i] == '>')
        result = handle_output_redir(input, i, tokens, preceded_by_space);
    else
        result = i + 1;  // Unexpected character, skip it
    
    return (result);
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
    
    preceded_by_space = 0;
    if (i > 0 && is_whitespace(input[i - 1]))
        preceded_by_space = 1;
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
    if (quote == '\'')
        add_token(tokens, create_token(TOKEN_SINGLE_QUOTE, word, preceded_by_space));
    else
        add_token(tokens, create_token(TOKEN_DOUBLE_QUOTE, word, preceded_by_space));
    return (i + 1);
}

/**
 * Handle word token in input with escape character support
 */
int handle_word(char *input, int i, t_token **tokens)
{
    int start;
    char *word;
    int preceded_by_space;
    int j = 0;
    char *processed_word;
    
    preceded_by_space = 0;
    if (i > 0 && is_whitespace(input[i - 1]))
        preceded_by_space = 1;
    
    start = i;
    while (input[i] && !is_whitespace(input[i]) && !is_special(input[i])
        && input[i] != '\'' && input[i] != '\"')
    {
        // Handle escape characters
        if (input[i] == '\\' && input[i+1])
            i++; // Skip the backslash and include the next character literally
        i++;
    }
    
    // Extract the raw word
    word = ft_substr(input, start, i - start);
    if (!word)
        return (-1);
    
    // Process escape sequences
    processed_word = malloc(ft_strlen(word) + 1);
    if (!processed_word)
    {
        free(word);
        return (-1);
    }
    
    i = start;
    while (input[i] && i < start + (int)ft_strlen(word))
    {
        if (input[i] == '\\' && input[i+1])
            processed_word[j++] = input[++i]; // Skip backslash, copy next char
        else
            processed_word[j++] = input[i];
        i++;
    }
    processed_word[j] = '\0';
    
    add_token(tokens, create_token(TOKEN_WORD, processed_word, preceded_by_space));
    free(word);
    
    return (i);
}

/**
 * Check if redirection token has a valid target
 */
static int	has_valid_redir_target(t_token *redir, t_token *next)
{
    if (!redir || !next)
        return (0);
        
    return (next->type == TOKEN_WORD || 
           next->type == TOKEN_SINGLE_QUOTE || 
           next->type == TOKEN_DOUBLE_QUOTE);
}

/**
 * Validate token syntax with improved error checking
 */
int	validate_syntax(t_token *tokens)
{
    t_token	*current;
    t_token	*next;

    if (!tokens)
        return (1); // Empty input is valid
        
    current = tokens;
    while (current)
    {
        next = current->next;
        
        // Pipes must not be at the beginning or end
        if (current->type == TOKEN_PIPE)
        {
            if (!next || current == tokens)
            {
                print_syntax_error("|", TOKEN_PIPE);
                return (0);
            }
                
            // Check for consecutive pipes
            if (next->type == TOKEN_PIPE)
            {
                print_syntax_error("|", TOKEN_PIPE);
                return (0);
            }
        }
        
        // Redirections must be followed by a valid target
        if (current->type == TOKEN_REDIR_IN || 
            current->type == TOKEN_REDIR_OUT ||
            current->type == TOKEN_REDIR_APPEND ||
            current->type == TOKEN_HEREDOC)
        {
            if (!has_valid_redir_target(current, next))
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
        if (input[i] == '\'' && !in_double_quote)
            in_single_quote = !in_single_quote;
        else if (input[i] == '\"' && !in_single_quote)
            in_double_quote = !in_double_quote;
        i++;
    }
    
    if (in_single_quote || in_double_quote)
    {
        ft_putstr_fd("minishell: syntax error: unclosed quotes\n", STDERR_FILENO);
        return (1);
    }
    
    return (0);
}

/**
 * Merge adjacent quoted tokens and words
 * This is important for handling commands like 'echo' "hello" 'world'
 */
void	merge_adjacent_quoted_tokens(t_token **tokens)
{
    t_token	*current;
    t_token	*next;
    t_token	*temp;
    char	*merged_value;
    
    if (!tokens || !*tokens)
        return ;
    
    current = *tokens;
    while (current && current->next)
    {
        next = current->next;
        
        // Check if we need to merge these tokens
        // They should be adjacent with no whitespace between them
        if (!next->preceded_by_space && 
            ((current->type == TOKEN_WORD || 
              current->type == TOKEN_SINGLE_QUOTE || 
              current->type == TOKEN_DOUBLE_QUOTE) && 
             (next->type == TOKEN_WORD ||
              next->type == TOKEN_SINGLE_QUOTE ||
              next->type == TOKEN_DOUBLE_QUOTE)))
        {
            // Merge the values
            merged_value = ft_strjoin(current->value, next->value);
            if (!merged_value)
                return ;  // Memory allocation error
            
            // Update current token
            free(current->value);
            current->value = merged_value;
            current->type = TOKEN_WORD;  // Merged tokens become words
            
            // Remove next token from list
            temp = next;
            current->next = next->next;
            free(temp->value);
            free(temp);
        }
        else
        {
            current = current->next;
        }
    }
}

/**
 * Process a token at the current position
 * All handler functions return the index of the next character to process
 * or -1 on error
 */
static int	process_token_part(char *input, int i, t_token **tokens, int *was_space)
{
    t_token	*last;

    if (is_whitespace(input[i]))
    {
        *was_space = 1;
        return (i + 1);
    }
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
    
    // Update preceded_by_space flag
    last = get_last_token(*tokens);
    if (last)
        last->preceded_by_space = *was_space;
    *was_space = 0;
    
    return (i);
}

/**
 * Process tokens from input string
 * Returns the list of tokens or NULL on error
 */
t_token	*process_tokens(char *input)
{
    t_token	*tokens;
    int		i;
    int		was_space;

    tokens = NULL;
    i = 0;
    was_space = 1;  // Assume leading space for the first token
    
    while (input[i])
    {
        i = process_token_part(input, i, &tokens, &was_space);
        if (i == -1)
        {
            free_token_list(tokens);
            ft_putstr_fd("minishell: syntax error: unclosed quote\n", 2);
            return (NULL);
        }
    }
    
    return (tokens);
}

/**
 * Tokenize the input string
 * This is the main entry point for the lexer
 */
t_token	*tokenize(char *input)
{
    t_token	*tokens;
    
    if (!input)
        return (NULL);
    
    // Check for unclosed quotes first    
    if (check_unclosed_quotes(input))
        return (NULL);
        
    // Generate raw tokens
    tokens = process_tokens(input);
    if (!tokens)
        return (NULL);
    
    // Post-process tokens: merge adjacent quoted segments
    merge_adjacent_quoted_tokens(&tokens);
        
    // Validate syntax
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
    
    printf("DEBUG: Processing input: '%s'\n", input);
    
    if (!input || !*input)
        return (NULL);
    
    // Add to history if non-empty
    if (*input)
        add_history(input);
    
    // Tokenize the input
    tokens = tokenize(input);
    if (!tokens) {
        printf("DEBUG: Tokenization failed\n");
        return (NULL);
    }
    
    printf("DEBUG: Tokenization successful\n");
    
    // Expand variables in tokens with word splitting
    tokens = expand_variables_in_tokens_with_splitting(tokens, shell);
    
    return (tokens);
}

#ifdef DEBUG
/**
 * Print token list for debugging (only compiled in DEBUG mode)
 */
void print_tokens(t_token *tokens)
{
    t_token *current = tokens;
    int i = 0;
    
    ft_putstr_fd("Token list:\n", 2);
    while (current)
    {
        ft_putstr_fd("  ", 2);
        ft_putnbr_fd(i++, 2);
        ft_putstr_fd(": type=", 2);
        ft_putnbr_fd(current->type, 2);
        ft_putstr_fd(" value='", 2);
        ft_putstr_fd(current->value, 2);
        ft_putstr_fd("' space=", 2);
        ft_putnbr_fd(current->preceded_by_space, 2);
        ft_putstr_fd("\n", 2);
        current = current->next;
    }
}
#endif