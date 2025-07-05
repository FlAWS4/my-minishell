# Utilities System Explained

## Overview

The utilities system is the toolbox of your minishell - a collection of helper functions, string manipulation tools, and support utilities that make everything else work smoothly. Think of it as the foundation that supports all the major systems.

## Why Utilities Are Important

Every complex system needs utility functions to:
- **Avoid code duplication** - Common operations in one place
- **Ensure consistency** - Same logic used everywhere
- **Improve maintainability** - Changes in one place affect whole system
- **Provide reliability** - Well-tested helper functions

## System Components

### 1. **String Manipulation** (`string_*.c`)
- String copying, joining, splitting
- Character handling and validation
- Memory-safe string operations

### 2. **Environment Utilities** (`env_utils.c`)
- Environment variable management
- Variable lookup and modification
- Environment array operations

### 3. **Token Utilities** (`token_utils.c`)
- Token creation and manipulation
- Token type checking
- Token list operations

### 4. **Initialization** (`init_*.c`)
- Shell initialization
- Environment setup
- File descriptor management

### 5. **Prompt Management** (`prompt.c`)
- Dynamic prompt generation
- Color and formatting
- User information display

### 6. **Memory Utilities** (`libft.c`)
- Custom library functions
- Memory allocation helpers
- Safe memory operations

### 7. **Input/Output** (`gnl.c`)
- Get next line functionality
- File reading utilities
- Input processing

## Detailed Breakdown

### 1. String Manipulation System

Your minishell has several string utility files that handle different aspects:

#### Basic String Operations (`string_utils.c`)

```c
// Safe string duplication
char *ft_strdup(const char *s1)
{
    char *dup;
    size_t len;
    size_t i;
    
    if (!s1)
        return (NULL);
    
    len = ft_strlen(s1);
    dup = malloc(sizeof(char) * (len + 1));
    if (!dup)
        return (NULL);
    
    i = 0;
    while (i < len)
    {
        dup[i] = s1[i];
        i++;
    }
    dup[i] = '\0';
    
    return (dup);
}

// Safe string joining
char *ft_strjoin(const char *s1, const char *s2)
{
    char *result;
    size_t len1, len2;
    size_t i, j;
    
    if (!s1 || !s2)
        return (NULL);
    
    len1 = ft_strlen(s1);
    len2 = ft_strlen(s2);
    
    result = malloc(sizeof(char) * (len1 + len2 + 1));
    if (!result)
        return (NULL);
    
    // Copy first string
    i = 0;
    while (i < len1)
    {
        result[i] = s1[i];
        i++;
    }
    
    // Copy second string
    j = 0;
    while (j < len2)
    {
        result[i + j] = s2[j];
        j++;
    }
    
    result[i + j] = '\0';
    return (result);
}
```

#### Advanced String Operations (`string_helpers.c`)

```c
// Split string by delimiter
char **ft_split(const char *s, char c)
{
    char **result;
    int word_count;
    int i, j, k;
    
    if (!s)
        return (NULL);
    
    // Count words
    word_count = count_words(s, c);
    
    // Allocate array
    result = malloc(sizeof(char *) * (word_count + 1));
    if (!result)
        return (NULL);
    
    // Extract words
    i = 0;
    k = 0;
    while (k < word_count)
    {
        // Skip delimiters
        while (s[i] == c)
            i++;
        
        // Find word end
        j = i;
        while (s[j] && s[j] != c)
            j++;
        
        // Extract word
        result[k] = ft_substr(s, i, j - i);
        if (!result[k])
        {
            free_string_array(result);
            return (NULL);
        }
        
        i = j;
        k++;
    }
    
    result[k] = NULL;
    return (result);
}

// Extract substring
char *ft_substr(const char *s, unsigned int start, size_t len)
{
    char *substr;
    size_t s_len;
    size_t i;
    
    if (!s)
        return (NULL);
    
    s_len = ft_strlen(s);
    
    // Check bounds
    if (start >= s_len)
        return (ft_strdup(""));
    
    // Adjust length if needed
    if (start + len > s_len)
        len = s_len - start;
    
    substr = malloc(sizeof(char) * (len + 1));
    if (!substr)
        return (NULL);
    
    i = 0;
    while (i < len)
    {
        substr[i] = s[start + i];
        i++;
    }
    substr[i] = '\0';
    
    return (substr);
}
```

#### String Validation (`string_tools.c`)

```c
// Check if string is numeric
int is_numeric_string(const char *str)
{
    int i = 0;
    
    if (!str || !str[0])
        return (0);
    
    // Handle optional sign
    if (str[0] == '+' || str[0] == '-')
        i++;
    
    // Check if no digits after sign
    if (!str[i])
        return (0);
    
    // Check each character is digit
    while (str[i])
    {
        if (!ft_isdigit(str[i]))
            return (0);
        i++;
    }
    
    return (1);
}

// Check if valid identifier
int is_valid_identifier(const char *str)
{
    int i;
    
    if (!str || !str[0])
        return (0);
    
    // First character must be letter or underscore
    if (!ft_isalpha(str[0]) && str[0] != '_')
        return (0);
    
    // Rest must be alphanumeric or underscore
    i = 1;
    while (str[i])
    {
        if (!ft_isalnum(str[i]) && str[i] != '_')
            return (0);
        i++;
    }
    
    return (1);
}
```

### 2. Environment Utilities

#### Environment Variable Lookup

```c
char *get_env_value(t_shell *shell, const char *var_name)
{
    int i = 0;
    size_t name_len;
    
    if (!shell || !shell->env || !var_name)
        return (NULL);
    
    name_len = ft_strlen(var_name);
    
    while (shell->env[i])
    {
        // Check if variable name matches
        if (ft_strncmp(shell->env[i], var_name, name_len) == 0 &&
            shell->env[i][name_len] == '=')
        {
            return (&shell->env[i][name_len + 1]);
        }
        i++;
    }
    
    return (NULL);
}
```

#### Environment Variable Update

```c
int update_env(t_shell *shell, const char *var_name, const char *value)
{
    char *new_var;
    char *temp;
    int pos;
    
    // Create new variable string
    temp = ft_strjoin(var_name, "=");
    if (!temp)
        return (1);
    
    new_var = ft_strjoin(temp, value);
    free(temp);
    if (!new_var)
        return (1);
    
    // Find existing variable
    pos = find_var_pos(var_name, shell);
    
    if (pos >= 0)
    {
        // Update existing
        free(shell->env[pos]);
        shell->env[pos] = new_var;
    }
    else
    {
        // Add new variable
        add_to_env(shell, new_var);
    }
    
    return (0);
}
```

#### Environment Array Management

```c
int add_to_env(t_shell *shell, char *new_var)
{
    char **new_env;
    int env_size;
    int i;
    
    // Count current environment size
    env_size = 0;
    while (shell->env[env_size])
        env_size++;
    
    // Allocate new array (one bigger)
    new_env = malloc(sizeof(char *) * (env_size + 2));
    if (!new_env)
        return (1);
    
    // Copy existing variables
    i = 0;
    while (i < env_size)
    {
        new_env[i] = shell->env[i];
        i++;
    }
    
    // Add new variable
    new_env[i] = new_var;
    new_env[i + 1] = NULL;
    
    // Replace old array
    free(shell->env);
    shell->env = new_env;
    
    return (0);
}
```

### 3. Token Utilities

#### Token Creation and Management

```c
t_token *create_token(t_token_type type, char *value)
{
    t_token *token;
    
    token = malloc(sizeof(t_token));
    if (!token)
        return (NULL);
    
    token->type = type;
    token->value = value;
    token->next = NULL;
    token->previous = NULL;
    
    return (token);
}

void add_token_to_list(t_token **head, t_token *new_token)
{
    t_token *current;
    
    if (!*head)
    {
        *head = new_token;
        return;
    }
    
    current = *head;
    while (current->next)
        current = current->next;
    
    current->next = new_token;
    new_token->previous = current;
}
```

#### Token Type Checking

```c
int is_redirection_token(t_token_type type)
{
    return (type == REDIR_IN || 
            type == REDIR_OUT || 
            type == REDIR_APPEND || 
            type == REDIR_HEREDOC);
}

int is_operator_token(t_token_type type)
{
    return (type == PIPE || is_redirection_token(type));
}

int needs_argument(t_token *token)
{
    return (is_redirection_token(token->type));
}
```

### 4. Prompt System

#### Dynamic Prompt Generation

```c
char *format_shell_prompt(t_shell *shell)
{
    char *user;
    char *cwd;
    char *home;
    char *prompt;
    char *temp;
    
    // Get user information
    user = get_env_value(shell, "USER");
    if (!user)
        user = "user";
    
    // Get current directory
    cwd = get_current_directory(shell);
    
    // Get home directory for ~ substitution
    home = get_env_value(shell, "HOME");
    if (home && ft_strncmp(cwd, home, ft_strlen(home)) == 0)
    {
        char *home_path = ft_strjoin("~", cwd + ft_strlen(home));
        free(cwd);
        cwd = home_path;
    }
    
    // Build prompt: [user@minishell path] $
    temp = ft_strjoin("[", user);
    temp = ft_strjoin_free(temp, "@minishell ");
    temp = ft_strjoin_free(temp, cwd);
    temp = ft_strjoin_free(temp, "] $ ");
    
    // Add colors
    prompt = ft_strjoin(BOLD_GREEN, temp);
    prompt = ft_strjoin_free(prompt, RESET);
    
    free(temp);
    free(cwd);
    
    return (prompt);
}
```

#### Color and Formatting

```c
// Color definitions
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"

// Compound colors
#define BOLD_RED    "\033[1;31m"
#define BOLD_GREEN  "\033[1;32m"
#define BOLD_BLUE   "\033[1;34m"
#define BOLD_YELLOW "\033[1;33m"
```

### 5. Initialization Utilities

#### Shell Initialization

```c
void init_shell_fds(t_shell *shell)
{
    // Save original file descriptors
    shell->saved_stdin = dup(STDIN_FILENO);
    shell->saved_stdout = dup(STDOUT_FILENO);
    shell->saved_stderr = dup(STDERR_FILENO);
    
    // Initialize memory manager
    shell->memory_manager = NULL;
    
    // Initialize command history
    using_history();
}

void init_shell_state(t_shell *shell)
{
    shell->commands = NULL;
    shell->tokens = NULL;
    shell->heredoc_interupt = 0;
    shell->default_path = NULL;
}
```

#### Welcome Display

```c
void ft_display_welcome(void)
{
    ft_putstr_fd(BOLD_CYAN, STDOUT_FILENO);
    ft_putstr_fd(" __  __ _       _     _          _ _ \n", STDOUT_FILENO);
    ft_putstr_fd("|  \\/  (_)     (_)   | |        | | |\n", STDOUT_FILENO);
    ft_putstr_fd("| \\  / |_ _ __  _ ___| |__   ___| | |\n", STDOUT_FILENO);
    ft_putstr_fd("| |\\/| | | '_ \\| / __| '_ \\ / _ \\ | |\n", STDOUT_FILENO);
    ft_putstr_fd("| |  | | | | | | \\__ \\ | | |  __/ | |\n", STDOUT_FILENO);
    ft_putstr_fd("|_|  |_|_|_| |_|_|___/_| |_|\\___|_|_|\n", STDOUT_FILENO);
    ft_putstr_fd(RESET, STDOUT_FILENO);
    ft_putstr_fd("\n", STDOUT_FILENO);
    ft_putstr_fd(BOLD_WHITE "Welcome to Minishell! Type 'help' to see available commands.\n" RESET, STDOUT_FILENO);
    ft_putstr_fd("\n", STDOUT_FILENO);
}
```

### 6. Memory Management Utilities

#### Safe Memory Operations

```c
void *ft_memset(void *b, int c, size_t len)
{
    unsigned char *ptr = (unsigned char *)b;
    size_t i = 0;
    
    while (i < len)
    {
        ptr[i] = (unsigned char)c;
        i++;
    }
    
    return (b);
}

void *ft_memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    size_t i = 0;
    
    if (!dst && !src)
        return (NULL);
    
    while (i < n)
    {
        d[i] = s[i];
        i++;
    }
    
    return (dst);
}
```

#### Array Management

```c
void free_string_array(char **array)
{
    int i = 0;
    
    if (!array)
        return;
    
    while (array[i])
    {
        free(array[i]);
        i++;
    }
    
    free(array);
}

int count_array_elements(char **array)
{
    int count = 0;
    
    if (!array)
        return (0);
    
    while (array[count])
        count++;
    
    return (count);
}
```

### 7. File and I/O Utilities

#### Get Next Line

```c
char *get_next_line(int fd, int cleanup_mode)
{
    static char *buffer;
    char *line;
    char *temp;
    int bytes_read;
    
    if (cleanup_mode)
    {
        if (buffer)
        {
            free(buffer);
            buffer = NULL;
        }
        return (NULL);
    }
    
    if (fd < 0 || BUFFER_SIZE <= 0)
        return (NULL);
    
    // Read and process data
    line = extract_line(&buffer);
    
    while (!line && (bytes_read = read(fd, temp_buffer, BUFFER_SIZE)) > 0)
    {
        temp_buffer[bytes_read] = '\0';
        buffer = join_and_free(buffer, temp_buffer);
        line = extract_line(&buffer);
    }
    
    return (line);
}
```

#### File Operations

```c
int is_directory(const char *path)
{
    struct stat path_stat;
    
    if (stat(path, &path_stat) != 0)
        return (0);
    
    return (S_ISDIR(path_stat.st_mode));
}

int file_exists(const char *path)
{
    return (access(path, F_OK) == 0);
}

int is_executable(const char *path)
{
    return (access(path, X_OK) == 0);
}
```

### 8. Character and Type Utilities

#### Character Classification

```c
int ft_isalpha(int c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

int ft_isdigit(int c)
{
    return (c >= '0' && c <= '9');
}

int ft_isalnum(int c)
{
    return (ft_isalpha(c) || ft_isdigit(c));
}

int ft_isspace(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || 
            c == '\r' || c == '\f' || c == '\v');
}
```

#### String Conversion

```c
int ft_atoi(const char *str)
{
    int result = 0;
    int sign = 1;
    int i = 0;
    
    // Skip whitespace
    while (ft_isspace(str[i]))
        i++;
    
    // Handle sign
    if (str[i] == '-' || str[i] == '+')
    {
        if (str[i] == '-')
            sign = -1;
        i++;
    }
    
    // Convert digits
    while (ft_isdigit(str[i]))
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    
    return (result * sign);
}
```

### 9. Utility Integration

#### How Utilities Support Main Systems

**Parser Uses String Utilities**:
```c
// In tokenizer
tokens = ft_split(input, ' ');
token_value = ft_substr(input, start, length);
```

**Executor Uses Environment Utilities**:
```c
// In command execution
char *path = get_env_value(shell, "PATH");
update_env(shell, "PWD", new_directory);
```

**Error System Uses Display Utilities**:
```c
// In error handling
error("command", filename, strerror(errno));
warning("export", variable, "already exists");
```

### 10. Performance and Safety

#### Memory Safety

All utility functions:
- Check for NULL pointers
- Validate input parameters
- Handle edge cases gracefully
- Clean up allocated memory

#### Error Handling

```c
char *safe_strdup(const char *s)
{
    char *dup;
    
    if (!s)
        return (NULL);
    
    dup = ft_strdup(s);
    if (!dup)
    {
        error("strdup", NULL, "memory allocation failed");
        return (NULL);
    }
    
    return (dup);
}
```

#### Efficiency

- Functions are optimized for common use cases
- Minimal memory allocations
- Efficient string operations
- Reusable code patterns

This utilities system provides the solid foundation that makes all other parts of your minishell work reliably and efficiently!
