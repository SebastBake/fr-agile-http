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
SRC=src/chlist.o src/tcpserver.o src/httpserver.o src/main.o

compile: $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(OUT);

clean:
	rm -f src/*.o; rm $(OUT);

test: compile
	test_script.sh $(OUT) 8080
