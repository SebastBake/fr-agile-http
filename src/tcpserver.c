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

#define LISTENQ_LEN 20
#define PORT_DIGITS 6

#define SOCK_ERR -1
#define PTHREAD_SUCCESS 0
#define GETADDRINFO_SUCCESS 0
#define ERR_MSG "A TCP server error occurred"


typedef struct {
	app_t* app;
	int fd;
	pthread_t* thread;
} runapp_arg_t;

struct addrinfo* gethostaddrinfo(unsigned short port);
struct addrinfo gethostaddrinfohints();
int initserver(unsigned short port);
void socketerrcheck(int status);
void* runapp(void* arg);
void startappthread(int fd, app_t* app);


/***************************************************************************
 *    Public functions
 */


// Start the server and allow the app to handle connections
void servetcp(unsigned short port, app_t* app) {

	int listen_fd, conn_fd;
	struct sockaddr_storage conn_addr;
	unsigned int conn_len = sizeof(conn_addr);

	listen_fd = initserver(port);
	for (;;) {
		conn_fd = accept(listen_fd, (struct sockaddr*) &conn_addr, &conn_len);
		socketerrcheck(conn_fd);
		startappthread(conn_fd, app);
	}
}
 

/***************************************************************************
 *    Private functions
 */


// Gets the hints to get the socket address info for the hosting machine
struct addrinfo gethostaddrinfohints() {
	struct addrinfo hints;
	memset(&hints, 0, (int)sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	return hints;
}


// Handles errors returned by socket.h functions
void socketerrcheck(int status) {

	if (status == SOCK_ERR) {
		perror(ERR_MSG);
		exit(EXIT_FAILURE);
	}
}


// Gets the socket address info for the hosting machine
struct addrinfo* gethostaddrinfo(unsigned short port) {

	int status;
	char portstr[PORT_DIGITS];
	struct addrinfo* hostaddrinfo;
	struct addrinfo addrinfohints = gethostaddrinfohints();

	sprintf(portstr ,"%d", port); // convert port int to str

	status = getaddrinfo(NULL, portstr, &addrinfohints, &hostaddrinfo);
	if (status != GETADDRINFO_SUCCESS) {
		printf("%s", gai_strerror(status));
		exit(EXIT_FAILURE);
	}

	// TODO: walk through linked list to get a correct result
	return hostaddrinfo;
}


// Initialises the server
int initserver(unsigned short port) {

	int fd;
	int bind_status, listen_status;
	struct addrinfo* ai = gethostaddrinfo(port);

	fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	socketerrcheck(fd);

	bind_status = bind(fd, ai->ai_addr, ai->ai_addrlen);
	socketerrcheck(bind_status);

	listen_status = listen(fd, LISTENQ_LEN);
	socketerrcheck(listen_status);

	freeaddrinfo(ai);
	return fd;
}


// Runs the app. NOTE: argument and return types are void* because this
// function is called by pthread_create(...)
void* runapp(void* arg_ptr) {

	runapp_arg_t* arg = arg_ptr;
	arg->app->run(arg->fd, arg->app->args);

	shutdown(arg->fd, SHUT_RDWR);
	close(arg->fd);
	//free(arg->thread); // This line causes seg fault??
	free(arg);
	return NULL;
}


// Start the app in new thread
void startappthread(int fd, app_t* app) {

	int success;
	runapp_arg_t* runapp_arg = (runapp_arg_t*) malloc(sizeof(runapp_arg_t));
	assert(runapp_arg != NULL);

	runapp_arg->app = app;
	runapp_arg->fd = fd;
	runapp_arg->thread = (pthread_t*) malloc(sizeof(pthread_t));
	assert(runapp_arg->thread!=NULL);
	success = pthread_create(runapp_arg->thread, NULL, &runapp, runapp_arg);
	if(!success) {
		shutdown(arg->fd, SHUT_RDWR);
		close(arg->fd);
		free(runapp_arg);
	}
}
