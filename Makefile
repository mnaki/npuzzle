CC = c++
CFLAGS = -Wextra -Wall -Ofast -std=c++14
EXEC_NAME = npuzzle
INCLUDES =
LIBS =
OBJ_FILES = Game.o  heuristics.o  main.o  Position.o  statecomp.o  State.o

all : $(EXEC_NAME)

clean :
  rm $(EXEC_NAME) $(OBJ_FILES)

$(EXEC_NAME) : $(OBJ_FILES)
  $(CC) -o $(EXEC_NAME) $(OBJ_FILES) $(LIBS)

%.o: %.cpp
  $(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

%.o: %.cc
  $(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

%.o: %.c
  gcc $(CFLAGS) $(INCLUDES) -o $@ -c $<