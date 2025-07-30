CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g -Iinclude

SRC_DIR = src
INC_DIR = include

CXXFILES = $(SRC_DIR)/main.cpp \
           $(SRC_DIR)/LoopDeLoop.cpp \
           $(SRC_DIR)/SockItToMe.cpp \
           $(SRC_DIR)/SocketZilla.cpp \
					 $(SRC_DIR)/Client.cpp

CXXOBJ = $(CXXFILES:.cpp=.o)

program = ircserv

all: $(program)

$(program): $(CXXOBJ)
	$(CXX) $(CXXFLAGS) $(CXXOBJ) -o $(program)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DIR)/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(CXXOBJ)

fclean: clean
	rm -f $(program)

re: fclean all

.PHONY: clean fclean re all

