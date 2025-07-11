/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 18:48:08 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 18:48:08 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	ft_putendl_fd(char *s, int fd)
{
	if (fd < 0)
		return ;
	if (s)
		ft_putstr_fd(s, fd);
	ft_putchar_fd('\n', fd);
}

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

int	ft_putchar_fd(char c, int fd)
{
	if (write(fd, &c, 1) == -1)
		return (-1);
	return (0);
}

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
	printf("Type %s'help'%s to see available commands.%s\n\n", \
		BOLD_WHITE, YELLOW, RESET);
}

int	ft_atoi(const char *nptr)
{
	int	nb;
	int	i;
	int	signe;

	nb = 0;
	i = 0;
	signe = 1;
	if (!nptr)
		return (0);
	while ((nptr[i] >= 9 && nptr[i] <= 13) || nptr[i] == ' ')
		i++;
	if (nptr[i] == '+' || nptr[i] == '-')
	{
		if (nptr[i] == '-')
			signe = -signe;
		i++;
	}
	while (nptr[i] >= '0' && nptr[i] <= '9')
	{
		nb = nb * 10 + (nptr[i] - '0');
		i++;
	}
	return (nb * signe);
}
