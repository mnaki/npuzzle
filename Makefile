SRC = main.cpp
NAME = npuzzle

all: $(NAME)

$(NAME):
	c++ -std=c++14 $(SRC) -o $(NAME)
	@echo "./npuzzle manhattan|hamming|maison|dijkstra [puzzle.txt]|[WIDTH HEIGHT]"

clean:
	rm -rf $(NAME)

fclean: clean

re: fclean all