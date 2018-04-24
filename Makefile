CC = c++
CFLAGS = -Wextra -Wall -Ofast -std=c++14
NAME = npuzzle
INCLUDES =
LIBS =
OBJ_FILES = Game.o  heuristics.o  main.o  Position.o  statecomp.o  State.o

all : $(NAME)

clean :
	rm $(NAME) $(OBJ_FILES)

$(NAME) : $(OBJ_FILES)
	$(CC) -o $(NAME) $(OBJ_FILES) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<