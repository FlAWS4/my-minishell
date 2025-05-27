/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:42:20 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/26 23:49:23 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if character is a digit
 */
int	ft_isdigit(int c)
{
    return (c >= '0' && c <= '9');
}

/**
 * Output string followed by newline to file descriptor
 */
void	ft_putendl_fd(char *s, int fd)
{
    ft_putstr_fd(s, fd);
    ft_putchar_fd('\n', fd);
}

/**
 * Concatenate strings with size limit
 */
size_t	ft_strlcat(char *dst, const char *src, size_t size)
{
    size_t	i;
    size_t	d;
    size_t	s;
    size_t	j;

    i = 0;
    j = 0;
    d = ft_strlen(dst);
    s = ft_strlen(src);
    if (size < d || size == 0)
        return (s + size);
    j = d;
    while (src[i] != '\0' && j < (size - 1))
    {
        dst[j] = src[i];
        i++;
        j++;
    }
    dst[j] = '\0';
    return (d + s);
}

/**
 * Output character to file descriptor
 */
void	ft_putchar_fd(char c, int fd)
{
    write(fd, &c, 1);
}

/**
 * Display welcome banner for minishell
 */
void	ft_display_welcome(void)
{
    printf("%s", BOLD_CYAN);
    printf(" __  __ _       _     _          _ _ \n");
    printf("|  \\/  (_)     (_)   | |        | | |\n");
    printf("| \\  / |_ _ __  _ ___| |__   ___| | |\n");
    printf("| |\\/| | | '_ \\| / __| '_ \\ / _ \\ | |\n");
    printf("| |  | | | | | | \\__ \\ | | |  __/ | |\n");
    printf("|_|  |_|_|_| |_|_|___/_| |_|\\___|_|_|\n");
    printf("%s\n", RESET);
    
    printf("%sWelcome to Minishell! Type %s'help'%s to see available commands.%s\n\n", 
           YELLOW, BOLD_WHITE, YELLOW, RESET);
}

/**
 * Check if character is whitespace
 */
int	is_whitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}