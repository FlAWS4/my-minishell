# How Environment Variables Are Inherited in Minishell

## The Big Picture: Environment Inheritance

When you run `./minishell`, it doesn't create environment variables out of thin air. Instead, it **inherits** them from the shell that launched it (like bash). This is a fundamental concept in Unix/Linux systems called **environment inheritance**.

## Step-by-Step Process

### 1. The Operating System Passes Environment Variables

When you type `./minishell` in your terminal, the operating system automatically passes three things to your program:
- `argc` (argument count)
- `argv` (argument values)  
- `envp` (environment pointer) ← **This is the key!**

```c
int main(int argc, char **argv, char **envp)
```

The `envp` parameter is an array of strings, where each string is in the format `"VARIABLE_NAME=value"`.

### 2. Environment Initialization Process

In `main.c`, the first thing your minishell does is:

```c
shell.env = init_env(envp, &shell);
```

This calls the `init_env` function which:

1. **Checks if PATH exists**: If the parent environment has a PATH variable, it uses the full environment
2. **Creates minimal environment**: If no PATH exists, it creates a basic environment with just PWD and _

### 3. Environment Copying Process

The `init_env` function calls `get_env(envp, shell)` which:

1. **Counts environment variables**:
   ```c
   while (envp[env_vars])
       env_vars++;
   ```

2. **Allocates memory** for the shell's environment array:
   ```c
   shell->env = allocate_managed_memory(&shell->memory_manager, 
       sizeof(char *) * (env_vars + 1), MEM_ERROR_FATAL, NULL);
   ```

3. **Duplicates each environment variable**:
   ```c
   static int duplicate_env_vars(char **envp, t_shell *shell)
   {
       int i = 0;
       while (envp[i])
       {
           shell->env[i] = managed_string_copy(&shell->memory_manager, envp[i]);
           i++;
       }
       shell->env[i] = NULL;
       return (1);
   }
   ```

### 4. What Gets Inherited

Your minishell inherits **everything** from the parent shell:
- `PATH` (where to find commands)
- `HOME` (your home directory)
- `USER` (your username)
- `PWD` (current working directory)
- `SHELL` (the shell path)
- `TERM` (terminal type)
- `LANG` (language settings)
- And many more...

### 5. Special Handling for SHLVL

After copying the environment, your minishell updates the `SHLVL` variable:

```c
if (shell->env)
    update_shell_lvl(shell);
```

This function:
1. Gets the current SHLVL value
2. Increments it by 1 (to show you're in a nested shell)
3. Updates the environment with the new value

## Visual Example

Let's say you have these environment variables in bash:
```
PATH=/usr/local/bin:/usr/bin:/bin
HOME=/home/username
USER=username
SHLVL=1
```

When you run `./minishell`:

1. **Operating System**: Passes these variables in `envp`
2. **Your minishell**: Copies them to its own `shell.env`
3. **SHLVL update**: Changes `SHLVL=1` to `SHLVL=2`
4. **Result**: Your minishell now has all the same variables!

## Where the Original Variables Come From

The original environment variables come from:

1. **System defaults**: Set by the OS during boot
2. **Login shell**: Set when you log in (from ~/.bashrc, ~/.profile, etc.)
3. **Parent processes**: Each shell inherits from its parent
4. **User modifications**: Variables you set with `export`

## Why This Matters

This inheritance system means:
- Your minishell automatically knows where to find commands (PATH)
- It knows your home directory (HOME)
- It has all your customizations and settings
- Programs you run from minishell will inherit these variables too

## Code Flow Summary

```
Parent Shell (bash)
    ↓ (has environment variables)
./minishell launched
    ↓ (OS passes envp)
main(argc, argv, envp)
    ↓
init_env(envp, &shell)
    ↓
get_env(envp, shell)
    ↓
duplicate_env_vars(envp, shell)
    ↓
Your minishell now has all environment variables!
```

This is why when you run `env` in your minishell, you see all those variables - they were inherited from the parent shell, not created by your minishell!
