# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/11 02:39:27 by mshariar          #+#    #+#              #
#    Updated: 2025/06/26 19:52:25 by hchowdhu         ###   ########.fr        #
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
				parser/init_tokens.c parser/create_token.c parser/parser_utils.c\
				parser/lexer.c parser/parser.c parser/tokens.c parser/parser_tokens.c \
				parser/parser_redirections.c parser/lexer_token.c parser/lexer_process.c \
				parser/expander.c parser/check_for_expand.c parser/create_expander.c\
				parser/expand_tokens.c parser/init_expand.c parser/multiple_expand.c\
				parser/parser_expand.c parser/add_env.c \
				executor/builtin_executor.c executor/command_path.c executor/process_utils.c executor/pipes_utils.c\
				executor/executor.c executor/redirections.c executor/pipes.c \
				executor/redir_files.c executor/redir_fds.c executor/redir_close.c \
				executor/commands.c executor/redirection_list.c \
				builtins/cd.c builtins/echo.c builtins/env.c builtins/exit.c \
				builtins/export.c builtins/pwd.c builtins/unset.c builtins/history.c \
				error_handling/error_display.c error_handling/error_free.c \
				error_handling/error_gc_utils.c  error_handling/error_gc.c\
				utils/expander_utils.c utils/libft.c utils/init_env.c \
				utils/env_utils.c utils/string_utils.c utils/shell_lifecycle.c \
				utils/string_helpers.c utils/string_tools.c utils/string_extras.c\
				utils/prompt.c utils/token_utils.c utils/gnl.c \
				utils/redir_utils.c utils/signal_utils.c utils/init_shell.c\
				signals/signals.c signals/signal_heredoc.c 

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
