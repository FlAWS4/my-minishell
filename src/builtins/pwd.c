/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 18:47:07 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 18:47:07 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Built-in command that prints the current working directory
 * Uses PWD environment variable when available to preserve symlinks
 */
int	builtin_pwd(t_shell *shell)
{
    char	*cwd;
    char	*env_pwd;

    env_pwd = get_env_value(shell, "PWD");
    if (env_pwd)
    {
        ft_putendl_fd(env_pwd, STDOUT_FILENO);
        g_exit_status = 0;
        return (0);
    }

    cwd = getcwd(NULL, 0);
    if (!cwd)
    {
        error("pwd", NULL, strerror(errno));
        g_exit_status = 1;
        return (1);
    }
    ft_putendl_fd(cwd, STDOUT_FILENO);
    free(cwd);
    g_exit_status = 0;
    return (0);
}

/**
 * ft_strrchr - Locates the last occurrence of a character in a string
 * @s: String to search in
 * @c: Character to locate
 * 
 * Returns: A pointer to the last occurrence of c in s, or NULL if not found
 */
char	*ft_strrchr(const char *s, int c)
{
    char	*last;
    int		i;

    last = NULL;
    i = 0;
    while (s[i])
    {
        if (s[i] == (char)c)
            last = (char *)&s[i];
        i++;
    }
    if ((char)c == '\0')
        return ((char *)&s[i]);
    return (last);
}

/**
 * Skips delimiter characters at current position
 */
static int	skip_delimiters(char *str, const char *delim)
{
    int	i;
    int	j;

    i = 0;
    while (str[i])
    {
        j = 0;
        while (delim[j] && str[i] != delim[j])
            j++;
        if (!delim[j])
            break ;
        i++;
    }
    return (i);
}

/**
 * Finds the end of the current token
 */
static int	find_token_end(char *str, const char *delim)
{
    int	i;

    i = 0;
    while (str[i] && !ft_strchr(delim, str[i]))
        i++;
    return (i);
}

/**
 * Tokenizes string by delimiter, remembers position between calls
 */
char	*ft_strtok(char *str, const char *delim)
{
    static char	*next_token;
    char		*token_start;
    int			i;

    if (str)
        next_token = str;
    if (!next_token || !delim)
        return (NULL);
    i = skip_delimiters(next_token, delim);
    if (!next_token[i])
        return (next_token = NULL);
    token_start = &next_token[i];
    i += find_token_end(&next_token[i], delim);
    if (!next_token[i])
        next_token = NULL;
    else
    {
        next_token[i] = '\0';
        next_token = &next_token[i + 1];
    }
    return (token_start);
}
