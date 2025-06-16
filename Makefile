# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/11 02:39:27 by mshariar          #+#    #+#              #
#    Updated: 2025/06/15 23:59:24 by mshariar         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ---------------------------------- COLORS ---------------------------------- #
BOLD	=	\033[1m
ITALIC	=	\033[3m
UNDERLN	=	\033[4m
RED		=	\033[31m
GREEN	=	\033[32m
YELLOW	=	\033[33m
BLUE	=	\033[34m
MAGENTA	=	\033[35m
CYAN	=	\033[36m
WHITE	=	\033[37m
RESET	=	\033[0m

# --------------------------------- COMMANDS --------------------------------- #
NAME    = minishell
CC      = cc
CFLAGS  = -Wall -Wextra -Werror -g3
RM      = rm -rf
MKDIR   = mkdir -p

# ---------------------------------- PATHS ----------------------------------- #
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# ---------------------------------- FILES ----------------------------------- #
SRC_FILES	=	main.c \
				parser/lexer.c parser/parser.c parser/tokens.c parser/parser_tokens.c \
				parser/parser_redirections.c parser/lexer_token.c parser/lexer_process.c \
				parser/expander.c \
				executor/executor.c executor/redirections.c executor/pipes.c \
				executor/commands.c executor/redirection_list.c executor/process_redir.c \
				builtins/cd.c builtins/echo.c builtins/env.c builtins/exit.c \
				builtins/export.c builtins/pwd.c builtins/unset.c builtins/history.c \
				utils/expander_utils.c utils/export_utils.c utils/libft.c \
				utils/env_utils.c utils/error_handling.c utils/string_utils.c \
				utils/string_utils2.c utils/string_utils3.c utils/exec_utils.c \
				utils/prompt.c utils/pipe_utils.c utils/init_utils.c utils/gnl.c \
				utils/redir_utils.c \
				signals/signals.c

SRCS =	$(addprefix $(SRC_DIR)/, $(SRC_FILES))
OBJS =	$(addprefix $(OBJ_DIR)/, $(SRC_FILES:.c=.o))
INCS =	-I$(INC_DIR)
LIBS =	-lreadline

# --------------------------------- TARGETS ---------------------------------- #
all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(GREEN)$(BOLD)Linking objects...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LIBS)
	@echo "$(GREEN)$(BOLD)✅ Minishell successfully compiled!$(RESET)"

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@if [ ! -f "$(NAME)" ] || [ $< -nt $@ ]; then \
        if [ ! -f "$(NAME)" ]; then \
            echo "$(BLUE)$(BOLD)----------------------------------------$(RESET)"; \
            echo "$(BLUE)$(BOLD)          MINISHELL COMPILATION        $(RESET)"; \
            echo "$(BLUE)$(BOLD)----------------------------------------$(RESET)"; \
        fi; \
        echo "$(CYAN)Compiling $<...$(RESET)"; \
        $(CC) $(CFLAGS) $(INCS) -c $< -o $@; \
    fi

clean:
	@echo "$(YELLOW)Cleaning object files...$(RESET)"
	@$(RM) $(OBJ_DIR)
	@echo "$(YELLOW)✅ Object files removed!$(RESET)"

fclean: clean
	@echo "$(YELLOW)Removing executable...$(RESET)"
	@$(RM) $(NAME)
	@echo "$(YELLOW)✅ Executable removed!$(RESET)"

re: fclean all

norm:
	@echo "$(MAGENTA)Running Norminette...$(RESET)"
	@norminette $(SRC_DIR) $(INC_DIR)

debug:	CFLAGS += -fsanitize=address
debug:	re
	@echo "$(RED)$(BOLD)🔍 Debug build with Address Sanitizer compiled!$(RESET)"

.PHONY:	all clean fclean re norm debug
