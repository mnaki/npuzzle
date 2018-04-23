SRC = main.cpp
NAME = npuzzle

all: $(NAME)

$(NAME):
	c++ -std=c++14 $(SRC) -Wextra -Wall -Ofast -o $(NAME)
	@echo "./npuzzle ntiles|manhattan|euclidean|dijkstra [puzzle.txt]|[WIDTH HEIGHT]"

clean:
	rm -rf $(NAME)

fclean: clean

re: fclean all