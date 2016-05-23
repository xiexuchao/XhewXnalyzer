DIR_SRC = ./src
DIR_OBJ = ./obj

SRC = $(wildcard ${DIR_SRC}/*.c)
DIR = $(notdir ${SRC})
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC}))

TARGET = main

CC = gcc
CFLAGS = -g -Wall

${TARGET}:${OBJ}
	@$(CC) $(OBJ) -o $@ -lrt

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	@$(CC) $(CFLAGS) -c $< -o $@ -lrt

.PHONY:all clean
clean:
	rm -rf ${DIR_OBJ}/*.o
	rm -rf ${TARGET}
all:
	@echo $(SRC)
	@echo $(DIR)
	@echo $(OBJ)
