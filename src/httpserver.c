/***************************************************************************
 *
 *   File        : httpserver.c
 *   Student Id  : 757931
 *   Name        : Sebastian Baker
 *
 ***************************************************************************/

#include "httpserver.h"
#include "tcpserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

// TODO: Write http parser to generate a http struct
// TODO: Write the part that decides what the response is
// TODO: Use currying to return a handler which serves the files specified
void tcp_stream_handler(unsigned int fd) {

	char buffer[256];
	int n;

	printf("DEBUG: Handling stream\n");
	while (1) {
		memset(buffer, '\0', 255);
		n = read(fd ,buffer, 255);
		if (n == 0){
			break;
		}
		write(fd, buffer ,255);
		printf("DEBUG: RECIEVED & SENT: %s\n", buffer);
	}
	return;
}


void httpserve(unsigned short port, char* files) {
	tcpserve(port, (handlerfunc_t*) (&tcp_stream_handler));
	return;
}