
BLUE=\033[0;38;5;123m
LIGHT_PINK = \033[0;38;5;200m
PINK = \033[0;38;5;198m
DARK_BLUE = \033[1;38;5;110m
GREEN = \033[1;32;111m
LIGHT_GREEN = \033[1;38;5;121m
LIGHT_RED = \033[0;38;5;110m
FLASH_GREEN = \033[33;32m
WHITE_BOLD = \033[37m

# nom de l'executable
NAME_S = Matt_daemon
NAME = Ben_AFK

# paths
SRC_PATH= srcs
OBJ_S_PATH= .objs_server
OBJ_PATH= .objs_client

INC_PATH= includes


INC= $(INC_PATH)/*

NAME_SRC= main.cpp server.cpp application.cpp option.cpp command.cpp client.cpp termcaps.cpp logging.cpp cryptage.cpp \

NAME_SRC_LEN	= $(shell echo -n $(NAME_SRC) | wc -w)
I				= 

OBJ_NAME		= $(NAME_SRC:.cpp=.o)


OBJS = $(addprefix $(OBJ_PATH)/,$(OBJ_NAME))
OBJS_S = $(addprefix $(OBJ_S_PATH)/,$(OBJ_NAME))


DEBUG_FLAG = #-Wall -Wextra -Werror -fsanitize=address

OPTIMISATION_FLAG = -ofast #-o3 #-ofast  pas d'interet pour l'instant

GPP			= g++ -std=c++11 $(OPTIMISATION_FLAG) $(DEBUG_FLAG)



all: $(NAME_S) $(NAME)

$(NAME_S) : $(OBJS_S)
	@$(GPP) $^ -o $@ -lncurses -lcrypto -lcryptopp
	@echo "	\033[2K\r$(DARK_BLUE)$(NAME_S):\t\t$(GREEN)loaded\033[0m"
	@$(eval I=$(shell echo $$((0))))

$(NAME) : $(OBJS)
	@$(GPP) $^ -o $@ -lncurses -lcrypto -lcryptopp
	@echo "	\033[2K\r$(DARK_BLUE)$(NAME):\t\t$(GREEN)loaded\033[0m"


$(OBJ_S_PATH)/%.o: $(SRC_PATH)/%.cpp $(INC) Makefile
	@mkdir $(OBJ_S_PATH) 2> /dev/null || true
	@$(GPP) -I $(INC_PATH) -c $< -o $@
	@$(eval I=$(shell echo $$(($(I)+1))))
	@printf "\033[2K\r${G}$(DARK_BLUE)>>\t\t\t\t$(I)/$(shell echo $(NAME_SRC_LEN)) ${N}$(BLUE)$<\033[36m \033[0m"

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp $(INC) Makefile
	@mkdir $(OBJ_PATH) 2> /dev/null || true
	@$(GPP) -I $(INC_PATH) -c $< -o $@  -D MATTDEAMONCLIENT
	@$(eval I=$(shell echo $$(($(I)+1))))
	@printf "\033[2K\r${G}$(DARK_BLUE)>>\t\t\t\t$(I)/$(shell echo $(NAME_SRC_LEN)) ${N}$(BLUE)$<\033[36m \033[0m"


clean:
ifeq ("$(wildcard $(OBJ_PATH))", "")
else
	@rm -f $(OBJS)
	@rm -f $(OBJS_S)
	@rmdir $(OBJ_PATH) 2> /dev/null || true
	@rmdir $(OBJ_S_PATH) 2> /dev/null || true

	@printf "\033[2K\r$(DARK_BLUE)$(NAME) objects:\t$(LIGHT_PINK)removing\033[36m \033[0m\n"
	@printf "\033[2K\r$(DARK_BLUE)$(NAME_S) objects:\t$(LIGHT_PINK)removing\033[36m \033[0m\n"

endif


fclean: clean
ifeq ("$(wildcard $(NAME))", "")
else
	@rm -f $(NAME)
	@rm -f $(NAME_S)
	@printf "\033[2K\r$(DARK_BLUE)$(NAME):\t\t$(PINK)removing\033[36m \033[0m\n"
	@printf "\033[2K\r$(DARK_BLUE)$(NAME_S):\t\t$(PINK)removing\033[36m \033[0m\n"

endif

re: fclean all

.PHONY: all re clean fclean lib silent