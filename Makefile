# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/11 02:39:27 by mshariar          #+#    #+#              #
#    Updated: 2025/05/28 23:24:04 by mshariar         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror

SRCS = src/main.c \
       src/parser/lexer.c src/parser/parser.c src/parser/tokens.c src/parser/parser_tokens.c src/parser/parser_redirections.c\
       src/executor/executor.c src/executor/redirections.c src/executor/pipes.c src/executor/commands.c src/parser/lexer_token.c\
       src/builtins/cd.c src/builtins/echo.c src/builtins/env.c src/parser/lexer_process.c\
       src/builtins/exit.c src/builtins/export.c src/builtins/pwd.c src/utils/expander_utils.c src/builtins/history.c\
       src/builtins/unset.c src/utils/export_utils.c src/parser/expander.c src/utils/libft.c \
       src/utils/env_utils.c src/utils/error_handling.c src/utils/string_utils.c src/utils/string_utils2.c src/utils/string_utils3.c\
       src/signals/signals.c src/utils/exec_utils.c src/utils/prompt.c src/utils/pipe_utils.c src/utils/init_utils.c\
       src/utils/gnl.c src/utils/redir_utils.c src/executor/redirection_list.c src/executor/process_redir.c\


OBJS = $(SRCS:.c=.o)

INCS = -I./include

LIBS = -lreadline

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re