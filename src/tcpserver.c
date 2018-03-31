/***************************************************************************
 *
 *   File        : tcpserver.c
 *   Student Id  : 757931
 *   Name        : Sebastian Baker
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "tcpserver.h"

#define LISTENQ_LEN 5
#define SOCK_ERR -1
#define GETADDRINFO_SUCCESS 0
#define PORTSTR_LEN 32

#define GETADDRINFO_ERRSTR "Failed to get address info: "
#define SOCK_CREATE_ERRSTR "Failed to create socket: "
#define SOCK_BIND_ERRSTR "Failed to bind to socket: "

typedef struct run_handler_arg {
	handlerfunc_t* handler;
	unsigned int fd;
} run_handler_arg_t;

/* Private function declarations
 ***************************************************************************/

int init_server(unsigned short port);
struct addrinfo* host_addrinfo(unsigned short port);
void sock_err(unsigned short port);
void handle_connection(handlerfunc_t* handler,  unsigned int fd);
void *run_handler(void* args);


/* Private function implementations
 ***************************************************************************/

struct addrinfo* host_addrinfo(unsigned short port) {
	int addrinfo_status;
	struct addrinfo hints;
	struct addrinfo* ai; // socket address info
	char portstr[PORTSTR_LEN];

	memset(&hints, 0, (int)sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	sprintf(portstr ,"%d", port);
	printf("DEBUG: Getting host address info...\n");
	addrinfo_status = getaddrinfo(NULL, portstr, &hints, &ai);
	if (addrinfo_status != GETADDRINFO_SUCCESS) {
		printf("%s",gai_strerror(addrinfo_status));
		exit(EXIT_FAILURE);
	}

	// TODO: walk through linked list to get a correct result?

	return ai;
}


int init_server(unsigned short port) {
	//TODO: improve error handling

	int bind_err, listen_err;
	int listen_fd;
	struct addrinfo* ai;

	// Get address info
	ai = host_addrinfo(port);

	// Open Socket
	printf("DEBUG: Creating socket...\n"); // TODO: make a better debug function
	listen_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (listen_fd == SOCK_ERR) {
		perror(SOCK_CREATE_ERRSTR);
		exit(EXIT_FAILURE);
	}

	// Bind to port
	printf("DEBUG: Binding...\n");
	bind_err = bind(listen_fd, ai->ai_addr, ai->ai_addrlen);
	if (bind_err == SOCK_ERR) {
		perror(SOCK_BIND_ERRSTR);
		exit(EXIT_FAILURE);
	}

	// Start listening
	printf("DEBUG: Listening...\n");
	listen_err = listen(listen_fd, LISTENQ_LEN);
	if (listen_err == SOCK_ERR) {
		perror(SOCK_BIND_ERRSTR);
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(ai);

	return listen_fd;
}


void* run_handler(void* arg) {

	run_handler_arg_t* cast_arg = arg;
	unsigned int fd = cast_arg->fd;
	cast_arg->handler(fd);

	printf("DEBUG: Closing connection...\n");
	close(fd);
	return NULL;
}


/* Public function implementations
 ***************************************************************************/

// Starts the server
int tcpserve(unsigned short port, handlerfunc_t* handler) {

	int listen_fd;
	int conn_fd;
	struct sockaddr_storage conn_addr;
	unsigned int conn_len = sizeof(conn_addr);
	pthread_t newthread;
	run_handler_arg_t run_handler_arg;

	listen_fd = init_server(port);
	printf("DEBUG: Accepting...\n");
	while (1){
		conn_fd = accept(listen_fd, (struct sockaddr*) &conn_addr, &conn_len);
		if (conn_fd == SOCK_ERR) {
			perror("ERROR on accept");// TODO: fix error handling
			exit(EXIT_FAILURE);
		}
		// TODO: Clean up threading (Possibly move into another file)
		// TODO: (related) dynamically allocate args so they aren't
		// TODO: overwritten with multiple simultaneous connections
		// TODO: Fix naming of handlers and stuff - too 
		run_handler_arg.handler = handler;
		run_handler_arg.fd = conn_fd;
		pthread_create(&newthread, NULL, &run_handler, &run_handler_arg);
	}

	return 0;
}

