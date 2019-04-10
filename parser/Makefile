BIN = bin
INC = include
SRC = src
CC = gcc 
CFLAGS = -Wall -std=c11 -g 
LDFLAGS= -L.
UNAME_S := $(shell uname -s)
FOLDER = A3

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
    LIB := .dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIB := .so
endif

all: parser

parser: LinkedListAPI.o CalendarParser.o Helper.o
	${CC} -shared -o ../libcal.so ${BIN}/CalendarParser.o ${BIN}/Helper.o ${BIN}/LinkedListAPI.o 

CalendarParser.o: ${SRC}/CalendarParser.c 
	${CC} ${CFLAGS} -I${INC} -I${SRC} -c -fpic ${SRC}/CalendarParser.c -o ${BIN}/CalendarParser.o
 
list: LinkedListAPI.o
	$(CC) -shared -o ${BIN}/liblist.so ${BIN}/LinkedListAPI.o

LinkedListAPI.o: ${SRC}/LinkedListAPI.c
	$(CC) $(CFLAGS) -I${INC} -c -fpic ${SRC}/LinkedListAPI.c -o ${BIN}/LinkedListAPI.o

Helper.o: ${SRC}/Helper.c ${INC}/Helper.h
	${CC} ${CFLAGS} -I${INC} -I${SRC} -c -fpic ${SRC}/Helper.c -o ${BIN}/Helper.o 

clean: 
	rm ${BIN}/*.o
	rm ${BIN}/*.so

