CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = ircserv

INCLUDES_DIR = includes/
SRC_DIR = srcs/

INCLUDES_FILES = Channel.hpp \
				Client.hpp \
				Command.hpp \
				Server.hpp \

INCLUDES = $(addprefix $(INCLUDES_DIR), $(INCLUDES_FILES))

SRCS_FILES = main.cpp \
			Channel.cpp \
			Client.cpp \
			Command.cpp \
			Server.cpp \

SRCS = $(addprefix $(SRC_DIR), $(SRCS_FILES))

OBJS = $(SRCS:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJS) ${INCLUDES}
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(NAME)
	
%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	rm -rf $(OBJS)

fclean : clean
	rm -rf $(NAME)
	@rm -rf ircserv.dSYM
	@rm -rf report.xml

re : 
	make fclean 
	make all	

.PHONY : all clean fclean re
