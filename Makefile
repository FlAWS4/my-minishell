# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/11 02:39:27 by mshariar          #+#    #+#              #
#    Updated: 2025/05/14 20:43:02 by mshariar         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror

SRCS = src/main.c \
       src/parser/lexer.c src/parser/parser.c src/parser/tokens.c src/parser/parser_tokens.c src/parser/parser_redirections.c\
       src/executor/executor.c src/executor/redirections.c src/executor/pipes.c \
       src/builtins/cd.c src/builtins/echo.c src/builtins/env.c \
       src/builtins/exit.c src/builtins/export.c src/builtins/pwd.c \
       src/builtins/unset.c \
       src/utils/env_utils.c src/utils/error_handling.c src/utils/string_utils.c \
       src/signals/signals.c

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