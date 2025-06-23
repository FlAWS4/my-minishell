/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_handling.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:12 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 19:29:53 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

/**
 * error - Display formatted error message with color highlighting
 * @cmd: Command that caused the error (can be NULL)
 * @error_item: Item causing the error (can be NULL)
 * @msg: Error message to display
 *
 * Displays a formatted error message with color-coded components to improve
 * readability and error visibility.
 */
void	error(const char *cmd, const char *error_item, const char *msg)
{
    ft_putstr_fd(BOLD_WHITE "minishell: " RESET, STDERR_FILENO);
    if (cmd)
    {
        ft_putstr_fd(BOLD_BLUE, STDERR_FILENO);
        ft_putstr_fd(cmd, STDERR_FILENO);
        ft_putstr_fd(RESET ": ", STDERR_FILENO);
    }
    if (error_item)
    {
        ft_putstr_fd(BOLD_YELLOW, STDERR_FILENO);
        ft_putstr_fd(error_item, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
        if (msg)
            ft_putstr_fd(": ", STDERR_FILENO);
    }
    if (msg)
    {
        ft_putstr_fd(BOLD_RED, STDERR_FILENO);
        ft_putstr_fd((char *)msg, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
    }
    ft_putchar_fd('\n', STDERR_FILENO);
}

/**
 * error_quoted - Display error message with quoted item and color highlighting
 * @cmd: Command that caused the error (can be NULL)
 * @error_item: Item causing the error to be displayed in quotes (can be NULL)
 * @msg: Error message to display
 *
 * Similar to error() but puts the error_item in quotes for better visibility
 * of whitespace or special characters in the problematic item.
 */
void	error_quoted(const char *cmd, const char *error_item, const char *msg)
{
    ft_putstr_fd(BOLD_WHITE "minishell: " RESET, STDERR_FILENO);
    if (cmd)
    {
        ft_putstr_fd(BOLD_BLUE, STDERR_FILENO);
        ft_putstr_fd(cmd, STDERR_FILENO);
        ft_putstr_fd(RESET ": ", STDERR_FILENO);
    }
    if (error_item)
    {
        ft_putstr_fd(BOLD_YELLOW, STDERR_FILENO);
        ft_putchar_fd('\'', STDERR_FILENO);
        ft_putstr_fd(error_item, STDERR_FILENO);
        ft_putchar_fd('\'', STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
        if (msg)
            ft_putstr_fd(": ", STDERR_FILENO);
    }
    if (msg)
    {
        ft_putstr_fd(BOLD_RED, STDERR_FILENO);
        ft_putstr_fd((char *)msg, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
    }
    ft_putchar_fd('\n', STDERR_FILENO);
}

/**
 * warning - Display warning message with color formatting
 * @cmd: Command related to the warning (can be NULL)
 * @warning_item: Item that triggered the warning (can be NULL)
 * @msg: Warning message to display
 *
 * Displays a formatted warning message with yellow highlighting
 * to indicate non-critical issues.
 */
void	warning(const char *cmd, const char *warning_item, const char *msg)
{
    ft_putstr_fd(BOLD_WHITE "minishell: " RESET, STDERR_FILENO);
    if (cmd)
    {
        ft_putstr_fd(BOLD_BLUE, STDERR_FILENO);
        ft_putstr_fd(cmd, STDERR_FILENO);
        ft_putstr_fd(RESET ": ", STDERR_FILENO);
    }
    ft_putstr_fd(YELLOW "warning: " RESET, STDERR_FILENO);
    if (warning_item)
    {
        ft_putstr_fd(YELLOW, STDERR_FILENO);
        ft_putstr_fd(warning_item, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
        if (msg)
            ft_putstr_fd(": ", STDERR_FILENO);
    }
    if (msg)
    {
        ft_putstr_fd(YELLOW, STDERR_FILENO);
        ft_putstr_fd((char *)msg, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
    }
    ft_putchar_fd('\n', STDERR_FILENO);
}

void	handle_cmd_error(t_shell *shell, const char *cmd, const char *msg,
	int exit_code)
{
	error(NULL, cmd, msg);
	clean_and_exit_shell(shell, exit_code);
}

int	writable(int fd, const char *cmd_name)
{
	if (write(fd, "", 0) == -1)
	{
		error(cmd_name, NULL, strerror(errno));
		return (0);
	}
	return (1);
}


/**
 * Display heredoc EOF warning
 */

void	free_redirs(t_redir **redirs)
{
	t_redir	*current;
	t_redir	*next;

	current = *redirs;
	while (current)
	{
		next = current->next;
		if (current->file_or_del)
			free(current->file_or_del);
		if (current->heredoc_content)
			free(current->heredoc_content);
		free(current);
		current = next;
	}
	*redirs = NULL;
}

void	free_tokens_list(t_token **head)
{
	t_token	*current;
	t_token	*next;

	current = *head;
	while (current)
	{
		next = current->next;
		if (current->value)
			free(current->value);
		free(current);
		current = next;
	}
	*head = NULL;
}

void	free_array(void *ptr)
{
	int		i;
	char	**tab;

	if (!ptr)
		return ;
	tab = (char **)ptr;
	i = 0;
	while (tab[i])
	{
		free(tab[i]);
		i++;
	}
	free(tab);
}

void	free_command(t_command **cmds)
{
	t_command	*cmd;
	t_command	*next;

	if (!cmds || !*cmds)
		return ;
	cmd = *cmds;
	while (cmd)
	{
		next = cmd->next;
		if (cmd->args)
			free_array(cmd->args);
		if (cmd->redirs)
			free_redirs(&cmd->redirs);
		free(cmd);
		cmd = next;
	}
	*cmds = NULL;
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

char	*gc_substr(t_gc **gc, const char *s, unsigned int start, size_t len)
{
	size_t	i;
	size_t	y;
	char	*sub;

	i = 0;
	if (!s)
		return (NULL);
	if (start >= ft_strlen(s))
		return (gc_strdup(gc, ""));
	y = ft_strlen(s + start);
	if (y < len)
		len = y;
	sub = gc_malloc(gc, len + 1, GC_SOFT, NULL);
	if (!sub)
		return (NULL);
	while (i < len && s[start])
	{
		sub[i] = s[start];
		i++;
		start++;
	}
	sub[i] = '\0';
	return (sub);
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
void	*gc_malloc(t_gc **gc_list, size_t size, int fatal,
	void (*free_array)(void *))
{
	void	*ptr;

	ptr = malloc(size);
	if (!ptr)
	{
		ft_putstr_fd(ALLOCFAIL, STDERR_FILENO);
		if (fatal == GC_FATAL)
		{
			gc_free_all(gc_list);
			exit(EXIT_FAILURE);
		}
		return (NULL);
	}
	if (gc_add(gc_list, ptr, fatal, free_array))
	{
		ft_putstr_fd(ALLOCFAIL, STDERR_FILENO);
		if (fatal == GC_FATAL)
		{
			free(ptr);
			gc_free_all(gc_list);
			exit(EXIT_FAILURE);
		}
		return (free(ptr), NULL);
	}
	return (ptr);
}

int	gc_add(t_gc **gc_list, void *ptr, int fatal,
	void (*free_array)(void *))
{
	t_gc	*new_node;

	if (!ptr)
		return (1);
	new_node = malloc(sizeof(t_gc));
	if (!new_node)
	{
		ft_putstr_fd(ALLOCFAIL, STDERR_FILENO);
		if (fatal == GC_FATAL)
		{
			gc_free_all(gc_list);
			exit(EXIT_FAILURE);
		}
		return (1);
	}
	new_node->data = ptr;
	new_node->flag = fatal;
	new_node->free_array = free_array;
	new_node->next = *gc_list;
	*gc_list = new_node;
	return (0);
}

void	gc_free_all(t_gc **gc_list)
{
	t_gc	*current_node;
	t_gc	*next_node;

	if (!gc_list || !*gc_list)
		return ;
	current_node = *gc_list;
	while (current_node)
	{
		next_node = current_node->next;
		if (current_node->free_array)
			current_node->free_array(current_node->data);
		else
			free(current_node->data);
		free(current_node);
		current_node = next_node;
	}
	*gc_list = NULL;
}
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
