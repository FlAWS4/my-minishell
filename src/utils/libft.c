/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:42:20 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 18:23:17 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if character is a digit (0-9)
 * Returns 1 if true, 0 if false
 */
int	ft_isdigit(int c)
{
    return (c >= '0' && c <= '9');
}

/**
 * Output string followed by newline to file descriptor
 * Safely handles NULL strings
 */
void	ft_putendl_fd(char *s, int fd)
{
    if (fd < 0)
        return;
    if (s)
        ft_putstr_fd(s, fd);
    ft_putchar_fd('\n', fd);
}

/**
 * Concatenate strings with size limit
 * Returns total length of string it tried to create
 */
size_t	ft_strlcat(char *dst, const char *src, size_t size)
{
    size_t	i;
    size_t	d;
    size_t	s;

    if (!src)
        return (0);
    if (!dst && size == 0)
        return (ft_strlen(src));
        
    d = ft_strlen(dst);
    s = ft_strlen(src);
    
    if (size <= d)
        return (s + size);
        
    i = 0;
    while (src[i] && (d + i) < (size - 1))
    {
        dst[d + i] = src[i];
        i++;
    }
    dst[d + i] = '\0';
    return (d + s);
}

/**
 * Output character to file descriptor
 * Safely handles invalid file descriptors
 */
int	ft_putchar_fd(char c, int fd)
{
	if (write(fd, &c, 1) == -1)
		return (-1);
	return (0);
}

/**
 * Display welcome banner for minishell
 * Uses ANSI color codes for better visual appearance
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
    
    printf("%sWelcome to Minishell! ", YELLOW);
    printf("Type %s'help'%s to see available commands.%s\n\n", 
        BOLD_WHITE, YELLOW, RESET);
}


int	ft_atoi(const char *nptr)
{
    int	i;
    int	sign;
    long	result;

    i = 0;
    sign = 1;
    result = 0;
    while (is_whitespace(nptr[i]))
        i++;
    if (nptr[i] == '-' || nptr[i] == '+')
    {
        if (nptr[i] == '-')
            sign = -1;
        i++;
    }
    while (nptr[i] >= '0' && nptr[i] <= '9')
    {
        if (result > (INT_MAX - (nptr[i] - '0')) / 10)
            return (sign == 1 ? INT_MAX : INT_MIN);
            
        result = (result * 10) + (nptr[i] - '0');
        i++;
    }
    return ((int)(result * sign));
}
