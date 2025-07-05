# Parser and Lexer System Explained

## Overview

The parser and lexer system is the brain of your minishell - it takes the raw command string you type and converts it into a structured format that the executor can understand and run. Think of it as a translator that converts human language into computer instructions.

## The Big Picture: From Text to Executable Commands

```
Raw Input: "ls -la | grep txt > output.txt"
    ↓
Lexer: Breaks into tokens
    ↓
Parser: Creates command structure
    ↓
Expander: Handles variables and quotes
    ↓
Ready for execution!
```

## System Components

### 1. **Lexer** (Lexical Analysis)
- Breaks input into individual tokens
- Identifies operators, words, quotes, etc.
- Handles special characters and whitespace

### 2. **Parser** (Syntax Analysis)  
- Converts tokens into command structures
- Handles pipes, redirections, arguments
- Validates syntax and creates execution tree

### 3. **Expander** (Variable Expansion)
- Expands environment variables (`$HOME`, `$USER`)
- Handles quote removal and special expansions
- Processes command substitutions

## Detailed Breakdown

### 1. The Lexer: Breaking Down Input

#### What is Lexical Analysis?
The lexer takes your command string and breaks it into "tokens" - individual meaningful pieces.

**Example**: `ls -la | grep txt`
- Token 1: `ls` (WORD)
- Token 2: `-la` (WORD)  
- Token 3: `|` (PIPE)
- Token 4: `grep` (WORD)
- Token 5: `txt` (WORD)
- Token 6: EOF (End of input)

#### Token Types
Your lexer recognizes these token types:
```c
typedef enum e_token_type
{
    WORD,           // Regular words and arguments
    PIPE,           // | operator
    REDIR_IN,       // < operator
    REDIR_OUT,      // > operator
    REDIR_APPEND,   // >> operator
    REDIR_HEREDOC,  // << operator
    T_EOF           // End of input
} t_token_type;
```

#### How Tokenization Works

**Step 1: Character-by-Character Analysis**
```c
// Simplified tokenization process
while (*input)
{
    if (*input == ' ' || *input == '\t')
        skip_whitespace(&input);
    else if (*input == '|')
        add_pipe_token();
    else if (*input == '<')
        add_redirect_in_token();
    else if (*input == '>')
        add_redirect_out_token();
    else
        add_word_token();
}
```

**Step 2: Handle Special Cases**
- **Quotes**: `"hello world"` becomes single WORD token
- **Operators**: `>>` is different from `> >`
- **Escaping**: `\|` is literal pipe character

### 2. The Parser: Building Command Structure

#### What is Parsing?
The parser takes tokens and builds a structured representation of what to execute.

**Example**: `ls -la | grep txt`
```
Command 1:
├── args: ["ls", "-la"]
├── redirs: NULL
└── next: → Command 2

Command 2:
├── args: ["grep", "txt"]
├── redirs: NULL
└── next: NULL
```

#### The Command Structure
```c
typedef struct s_command
{
    char            **args;      // Command and arguments
    t_redir         *redirs;     // Redirections
    int             fd_in;       // Input file descriptor
    int             fd_out;      // Output file descriptor
    struct s_command *next;      // Next command in pipeline
    struct s_command *previous; // Previous command
} t_command;
```

#### Parsing Process

**Step 1: Count and Allocate**
```c
// Count how many words we have for this command
int args_nb = count_words(data);

// Allocate memory for argument array
cmd->args = malloc(sizeof(char *) * (args_nb + 1));
```

**Step 2: Fill Command Arguments**
```c
static int fill_command_args(t_shell *data, char **args)
{
    t_token *token = data->tokens;
    int i = 0;
    
    while (token && token->type != PIPE && token->type != T_EOF)
    {
        if (token->type == WORD && !is_redirection_operator(token->previous))
        {
            args[i++] = process_token_value(token);
        }
        token = token->next;
    }
    args[i] = NULL;
    return (0);
}
```

**Step 3: Handle Redirections**
```c
// For each redirection operator found
if (token->type == REDIR_OUT)
{
    // Create redirection structure
    t_redir *redir = create_redir(token);
    redir->type = REDIR_OUT;
    redir->file = get_next_word_token(token);
    add_redirs(&cmd->redirs, redir);
}
```

### 3. Pipeline Handling

#### What is a Pipeline?
A pipeline connects commands with `|` so output of one becomes input of the next.

**Example**: `ls | grep txt | sort`
```
┌─────┐    ┌──────────┐    ┌──────┐
│  ls │───▶│ grep txt │───▶│ sort │
└─────┘    └──────────┘    └──────┘
```

#### Pipeline Parsing Process

**Step 1: Count Pipes**
```c
int count_pipe(t_shell *data)
{
    t_token *token = data->tokens;
    int i = 1;  // At least one command
    
    while (token)
    {
        if (token->type == PIPE && token->next && token->next->type != PIPE)
            i++;
        token = token->next;
    }
    return (i);
}
```

**Step 2: Split by Pipes**
```c
int split_cmd_with_pipe(t_shell *data)
{
    int pipe_nb = count_pipe(data);
    t_command *cmds = NULL;
    t_token *token = data->tokens;
    
    while (pipe_nb-- && token && token->type != T_EOF)
    {
        // Create command from tokens up to next pipe
        data->tokens = token;
        create_command(data, &cmds);
        
        // Skip to next pipe
        while (token && token->type != PIPE && token->type != T_EOF)
            token = token->next;
        if (token && token->type == PIPE)
            token = token->next;
    }
    
    data->commands = cmds;
    return (0);
}
```

### 4. Redirection Handling

#### Types of Redirections
- `<` : Redirect input from file
- `>` : Redirect output to file (overwrite)
- `>>`: Redirect output to file (append)
- `<<`: Here document (read until delimiter)

#### Redirection Structure
```c
typedef struct s_redir
{
    t_redir_type    type;    // Type of redirection
    char            *file;   // File name or here-doc delimiter
    int             fd;      // File descriptor (when opened)
    struct s_redir  *next;   // Next redirection
} t_redir;
```

#### Redirection Parsing
```c
t_redir *init_redir(t_shell *data)
{
    t_token *token = data->tokens;
    t_redir *redirs = NULL;
    
    while (token && token->type != PIPE && token->type != T_EOF)
    {
        if (is_redirection_operator(token->type))
        {
            t_redir *redir = create_redir(token);
            redir->type = token->type;
            redir->file = get_next_word_token(token);
            add_redirs(&redirs, redir);
        }
        token = token->next;
    }
    
    return (redirs);
}
```

### 5. The Expansion System

#### What is Expansion?
Expansion processes special characters and variables in your command.

**Types of Expansion**:
1. **Variable expansion**: `$HOME` becomes `/home/user`
2. **Quote removal**: `"hello world"` becomes `hello world`
3. **Tilde expansion**: `~` becomes home directory
4. **Command substitution**: `$(date)` becomes command output

#### Variable Expansion Process
```c
char *expand_variable(char *str, t_shell *shell)
{
    char *result = NULL;
    char *var_name = NULL;
    char *var_value = NULL;
    
    if (str[0] == '$')
    {
        var_name = extract_variable_name(str);
        var_value = get_env_value(shell, var_name);
        if (var_value)
            result = ft_strdup(var_value);
        else
            result = ft_strdup("");  // Empty if not found
    }
    
    return (result);
}
```

#### Quote Handling
```c
char *process_quotes(char *str)
{
    char *result = NULL;
    int in_single_quote = 0;
    int in_double_quote = 0;
    
    // Process each character
    for (int i = 0; str[i]; i++)
    {
        if (str[i] == '\'' && !in_double_quote)
            in_single_quote = !in_single_quote;
        else if (str[i] == '"' && !in_single_quote)
            in_double_quote = !in_double_quote;
        else
            add_char_to_result(&result, str[i]);
    }
    
    return (result);
}
```

## Data Flow Through the Parser

```
Raw Input: "echo $HOME > file.txt"
    ↓
Lexer creates tokens:
    [WORD:"echo"] [WORD:"$HOME"] [REDIR_OUT:">"] [WORD:"file.txt"] [EOF]
    ↓
Parser creates command structure:
    Command {
        args: ["echo", "$HOME"]
        redirs: [REDIR_OUT: "file.txt"]
    }
    ↓
Expander processes variables:
    Command {
        args: ["echo", "/home/user"]
        redirs: [REDIR_OUT: "file.txt"]
    }
    ↓
Ready for execution!
```

## Error Handling in Parser

### Syntax Errors
The parser catches common syntax errors:
- `|` at beginning or end of input
- Missing file after redirection operator
- Unclosed quotes
- Invalid operator combinations

### Error Recovery
When an error occurs:
1. Display helpful error message
2. Clean up partially parsed structures
3. Return to main loop for next command
4. Don't crash the shell

## Integration with Other Systems

### With Lexer
- Parser receives tokens from lexer
- Requests more tokens as needed
- Handles token linking and navigation

### With Expander
- Parser creates initial command structure
- Expander modifies arguments and files
- Result is fully expanded command

### With Executor
- Parser provides structured commands
- Executor uses file descriptors and arguments
- Redirection information guides I/O setup

## Performance Considerations

### Memory Management
- Tokens are allocated and freed properly
- Command structures use managed memory
- Cleanup happens even on errors

### Efficiency
- Single-pass parsing where possible
- Minimal copying of strings
- Efficient token navigation

## Real-World Examples

### Simple Command
**Input**: `ls -la`
**Tokens**: `[WORD:"ls"] [WORD:"-la"] [EOF]`
**Command**: `{args: ["ls", "-la"], redirs: NULL}`

### Pipeline
**Input**: `ps aux | grep bash`
**Result**: Two linked commands with pipe connection

### Complex Redirection
**Input**: `cat < input.txt > output.txt`
**Result**: Command with input and output redirections

### Variable Expansion
**Input**: `echo "Hello $USER"`
**Result**: Command with expanded username

This parser system is the foundation that makes your shell understand and execute complex commands correctly!
