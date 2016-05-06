SRC = $(wildcard *.c)
DIR = $(notdir ${SRC})
OBJ = $(patsubst %.c,%.o,$(notdir ${SRC}))

TARGET = main

CC = gcc
CFLAGS = -g -Wall

${TARGET}:${OBJ}
	@$(CC) $(OBJ) -o $@ -lrt

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	@$(CC) $(CFLAGS) -c $< -o $@ -lrt

.PHONY:all clean
clean:
	rm -rf *.o
	rm -rf ${TARGET}
all:
	@echo $(SRC)
	@echo $(DIR)
	@echo $(OBJ)
