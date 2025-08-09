CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++20 -g -Iinclude

SRC_DIR = src
INC_DIR = include

CXXFILES = $(SRC_DIR)/main.cpp \
					$(SRC_DIR)/LoopDeLoop.cpp \
					$(SRC_DIR)/SockItToMe.cpp \
					$(SRC_DIR)/SocketZilla.cpp \
					$(SRC_DIR)/Client.cpp \
					$(SRC_DIR)/Channel.cpp

CXXOBJ = $(CXXFILES:.cpp=.o)

NAME = ircserv

all: $(NAME)

$(NAME): $(CXXOBJ)
	$(CXX) $(CXXFLAGS) $(CXXOBJ) -o $(NAME)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DIR)/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(CXXOBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re all

