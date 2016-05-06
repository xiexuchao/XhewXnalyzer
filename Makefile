DIR_SRC = ./src
DIR_OBJ = ./src

SRC = $(wildcard ${DIR_SRC}/*.c)
DIR = $(notdir ${SRC})
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC}))

TARGET = main

CC = gcc
CFLAGS = -g -Wall

${TARGET}:${OBJ}
	@$(CC) $(OBJ) -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY:all clean
clean:
	rm -rf ${DIR_OBJ}/*.o
	rm -rf ${TARGET}
all:
	@echo $(SRC)
	@echo $(DIR)
	@echo $(OBJ)
