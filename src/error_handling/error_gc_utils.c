/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_gc_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 15:50:05 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 15:50:05 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	int_len(long nb)
{
	int	len;

	len = 0;
	if (nb == 0)
		return (1);
	if (nb < 0)
	{
		nb = -nb;
		len++;
	}
	while (nb > 0)
	{
		nb /= 10;
		len++;
	}
	return (len);
}

char	*gc_itoa(t_gc **gc, int n)
{
	long	nb;
	int		len;
	char	*str;

	nb = (long)n;
	len = int_len(nb);
	str = gc_malloc(gc, sizeof(char) * (len + 1), GC_SOFT, NULL);
	if (!str)
		return (NULL);
	str[len--] = '\0';
	if (nb == 0)
		str[0] = '0';
	if (nb < 0)
	{
		str[0] = '-';
		nb = -nb;
	}
	while (nb > 0)
	{
		str[len--] = (nb % 10) + '0';
		nb /= 10;
	}
	return (str);
}

char	**gc_split(t_gc **gc, char *str, char c)
{
	char	**arr;

	arr = ft_split(str, c);
	if (!arr)
		return (NULL);
	if (gc_add(gc, arr, GC_SOFT, free_array))
		return (NULL);
	return (arr);
}

char	*gc_strjoin(t_gc **gc, const char *s1, const char *s2)
{
	size_t	i;
	size_t	y;
	char	*result;

	i = 0;
	y = 0;
	if (!s1 || !s2)
		return (NULL);
	result = gc_malloc(gc, ft_strlen(s1) + ft_strlen(s2) + 1, GC_SOFT, NULL);
	if (!result)
		return (NULL);
	while (s1[i])
	{
		result[i] = s1[i];
		i++;
	}
	while (s2[y])
	{
		result[i] = s2[y];
		i++;
		y++;
	}
	result[i] = '\0';
	return (result);
}

char	*gc_strdup(t_gc **gc, const char *s1)
{
	char	*dest;
	size_t	len;

	if (!s1)
		return (NULL);
	len = ft_strlen(s1) + 1;
	dest = gc_malloc(gc, len, GC_SOFT, NULL);
	if (!dest)
		return (NULL);
	ft_memcpy(dest, s1, len);
	return (dest);
}
