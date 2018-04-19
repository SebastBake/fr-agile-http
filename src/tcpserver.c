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
#include <signal.h>
#include "tcpserver.h"

#define LISTENQ_LEN 20
#define PORT_DIGITS 6

#define SOCK_ERR -1
#define GETADDRINFO_SUCCESS 0
#define ERR_MSG "A TCP server error occurred:\n"

typedef struct {
	appfunc_t* appfunc;
	void* appargs;
	int fd;
} thread_arg_t;

struct addrinfo* gethostaddrinfo(unsigned short port);
struct addrinfo gethostaddrinfohints();
int initserver(unsigned short port);
void socketerrcheck(int status);
void* thread_func(void* arg);
void sighandler(int signo);
void setsighandler();


/***************************************************************************
 *    Public functions
 */


// Start the server and allow the app to handle connections
void servetcp(unsigned short port, appfunc_t* appfunc, void* appargs) {

	int err;
	thread_arg_t* thread_arg;
	pthread_t* thread;
	int listen_fd, conn_fd;
	struct sockaddr_storage conn_addr;
	unsigned int conn_len = sizeof(conn_addr);

	listen_fd = initserver(port);
	for (;;) {
		conn_fd = accept(listen_fd, (struct sockaddr*) &conn_addr, &conn_len);
		socketerrcheck(conn_fd);
		
		thread_arg = (thread_arg_t*) malloc(sizeof(thread_arg_t));
		assert(thread_arg != NULL);
		thread_arg->appfunc = appfunc;
		thread_arg->appargs = appargs;
		thread_arg->fd = conn_fd;
		
		thread = (pthread_t*) malloc(sizeof(pthread_t));
		assert(thread!=NULL);

		err = pthread_create(thread, NULL, &thread_func, thread_arg);
		err = err || pthread_detach(*thread);
		if(err) {
			printf(ERR_MSG);
			shutdown(conn_fd, SHUT_RDWR);
			close(conn_fd);
			free(thread_arg);
		}
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
		printf("%s\n", gai_strerror(status));
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

	setsighandler();

	freeaddrinfo(ai);
	return fd;
}


// Runs the app. NOTE: argument and return types are void* because this
// function is called by pthread_create(...)
void* thread_func(void* arg) {
	
	thread_arg_t* thread_arg = arg;
	thread_arg->appfunc(thread_arg->fd, thread_arg->appargs);
	shutdown(thread_arg->fd, SHUT_WR);
	close(thread_arg->fd);
	free(arg);
	pthread_exit(NULL);
}

// Handles ctrl-c signal
void sighandler(int signo) {
	if (signo == SIGINT) {
		pthread_exit(NULL);
	}
}

// Handles ctrl-c signal
void setsighandler() {
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	assert(!sigaction(SIGINT, &sa, NULL));
}
