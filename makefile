#***************************************************************************
#
#   File        : makefile
#   Student Id  : 757931
#   Name        : Sebastian Baker
#
#***************************************************************************


CC=gcc
CFLAGS=-Wall -pthread
OUT=server
SRC=src/dynstr.o src/httpserver.o src/tcpserver.o src/main.o

all: clean compile clean

compile: $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(OUT);

clean:
	rm -f src/*.o;
