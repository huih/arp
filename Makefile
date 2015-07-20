
DIR_INC = ./include
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin
MKDIR = mkdir

TARGET = arp
CC=gcc  

BIN_TRAGET = ${DIR_BIN}/${TARGET}
SRC = $(wildcard ${DIR_SRC}/*.c)
OBJ = $(patsubst %.c, ${DIR_OBJ}/%.o, ${notdir ${SRC}})

CFLAGS= -g -Wall -I${DIR_INC}

all:$(DIR_OBJ) $(DIR_BIN) $(BIN_TRAGET)
${BIN_TRAGET}:${OBJ}
	$(CC) $(OBJ) -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) -c $< -o $@

${DIR_OBJ}:
	$(MKDIR) $@
	
${DIR_BIN}:
	$(MKDIR) $@
	
.PHONY:clean
clean:
	rm -rf ${DIR_OBJ} ${DIR_BIN}