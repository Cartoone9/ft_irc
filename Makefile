# ================================ COMPILER ================================== #
CC			= c++
CFLAGS		= -Wall -Wextra -Werror -std=c++98
NAME		= ircserv

# =============================== DIRECTORIES ================================ #
SRC_DIR		= srcs
INC_DIR		= hdrs
OBJ_DIR		= .obj
DEP_DIR		= .deps

# ================================= INCLUDES ================================= #
INCS		= -I$(INC_DIR)

# ================================= SOURCE FILES ============================= #
SRC_FILES	= BotLogs.class.cpp \
			  Connection.class.cpp \
			  Logs.class.cpp \
			  Rolls.class.cpp \
			  Client.struct.cpp \
			  Message.struct.cpp \
			  State.struct.cpp \
			  handlers_auth.cpp \
			  handlers_server.cpp \
			  handlers_router.cpp \
			  handlers_channel.cpp \
			  handlers_privmsg.cpp \
			  handlers_mode.cpp \
			  handlers_bot.cpp \
			  handlers_utils.cpp \
			  tests.cpp \
			  tests_auth.cpp \
			  tests_channel_modes.cpp \
			  tests_join.cpp \
			  tests_kick.cpp \
			  tests_mode.cpp \
			  tests_server.cpp \
			  tests_parsing.cpp \
			  tests_part.cpp \
			  tests_privmsg.cpp \
			  banner.cpp \
			  error.cpp \
			  socket.cpp \
			  main.cpp \
			  time.cpp \

# ================================= FILE PATHS =============================== #
SRC			= $(addprefix $(SRC_DIR)/, $(SRC_FILES))
OBJS		= $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.cpp=.o))
DEPS		= $(addprefix $(DEP_DIR)/, $(SRC_FILES:.cpp=.d))

# ================================= COLORS =================================== #
GREEN		= \033[0;92m
BOLD_GREEN	= \033[1;92m
CYAN		= \033[0;96m
YELLOW		= \033[0;93m
BLUE		= \033[0;34m
BOLD_BLUE	= \033[1;34m
BOLD_RED	= \033[1;31m
RESET		= \033[0m

# ================================== RULES =================================== #
all: $(NAME)

-include $(DEPS)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "Compilation of $(BOLD_GREEN)$(NAME)$(RESET) finished!"
	@./$(NAME) --test > /dev/null || echo "but $(BOLD_RED)tests failed$(RESET). For details: ./$(NAME) --test";

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(DEP_DIR)/%.d
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@
	@echo "$(CYAN)Compiled:$(RESET) $<"

$(DEP_DIR)/%.d: $(SRC_DIR)/%.cpp
	@mkdir -p $(DEP_DIR)
	@$(CC) $(CFLAGS) $(INCS) -MM -MT $(OBJ_DIR)/$*.o $< > $@

test: $(NAME)
	@echo "$(YELLOW)Running tests...$(RESET)"
	@./$(NAME) --test

clean:
	@rm -rf $(OBJ_DIR) $(DEP_DIR)
	@echo "$(YELLOW).obj/$(RESET) and $(YELLOW)dep/$(RESET) removed."

fclean: clean
	@rm -f $(NAME)
	@echo "$(YELLOW)$(NAME)$(RESET) removed."

re:
	@$(MAKE) fclean --no-print-directory
	@$(MAKE) all --no-print-directory

server: $(NAME)
	valgrind -q ./$(NAME) 6667 pass

client:
	irssi -c localhost -p 6667 -w pass

.PHONY: all clean fclean re test server client
