/***************************************************************************
 *
 *   File        : main.c
 *   Student Id  : 757931
 *   Name        : Sebastian Baker
 *
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "httpserver.h"

#define NUM_ARGS 3
#define INPUT_MSG "USAGE EXAMPLE: ./server [port number] [path to web root]\n"
#define STARTING_MSG "Starting server for files at %s, on port %d\n"

int main(int argc, char *argv[]) {

	// Parse Command Line Arguments
	if (argc != NUM_ARGS) {
		printf(INPUT_MSG);
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);
	char* files = argv[2];
	printf(STARTING_MSG, files, port);

	// Start server
	servehttp(port, files);

	return EXIT_SUCCESS;
}
