/***************************************************************************
 *
 *   File        : httpserver.c
 *   Student Id  : 757931
 *   Name        : Sebastian Baker
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "dynstr.h"
#include "httpserver.h"
#include "tcpserver.h"

#define METHOD_GET "GET"
#define HTTP_VERSION "HTTP/1.0"
#define CONTENT_LEN_HEADER "Content-Length: %d\r\n"
#define MIMETYPE_HEADER "Content-Type: %s\r\n"
#define SERVER_HEADER "Server: Hal9000\r\n"
#define DATE_HEADER "Date: Now, Today, This Year\r\n"

#define MIMETYPE_JPG_X ".jpg"
#define MIMETYPE_JPEG_X ".jpeg"
#define MIMETYPE_HTML_X ".html"
#define MIMETYPE_PLAIN_X ".txt"
#define MIMETYPE_JS_X ".js"
#define MIMETYPE_CSS_X ".css"

#define MIMETYPE_JPEG "image/jpeg"
#define MIMETYPE_HTML "text/html"
#define MIMETYPE_PLAIN "text/plain"
#define MIMETYPE_JS "application/javascript"
#define MIMETYPE_CSS "text/css"
#define MIMETYPE_DEFAULT MIMETYPE_PLAIN

#define STATUS_LEN 64
#define STATUS_400 400
#define STATUS_404 404
#define STATUS_200 200
#define STATUS_501 501
#define STATUS_414 414
#define STATUS_400_PRINT "400 Bad Request"
#define STATUS_404_PRINT "404 Not Found"
#define STATUS_200_PRINT "200 OK"
#define STATUS_501_PRINT "501 Not Implemented"
#define STATUS_414_PRINT "414 URI Too Long"

#define DOT '.'
#define SPACE " "
#define READ_BINARY "rb"
#define REQINE_PARSE 2
#define INT_DIGITS 10
#define METHOD_LEN 8
#define MIMETYPE_MAXLEN strlen(MIMETYPE_JS) 
#define HTTP_VERSION_LEN 16
#define URL_LEN 256

typedef struct {
	char* files;
} appargs_t;

void append_date_and_server_headers(dynstr_t* s);
void append_contentlen_header(dynstr_t* s, int contentlen);
void append_mimetype_header(dynstr_t* s, char* filename);
void append_statusline(dynstr_t* s, char* status_str);

void http_err(int fd, int errcode);
void send_file(int fd, char* filename);
void appfunc(int fd, void* args);

int read_request_line(int fd, char* methodbuf, char* urlbuf);
char* http_err_to_msg(int errcode);


/***************************************************************************
 *    Public functions
 */

// Serves files over http
// files -- root folder from which to serve files
// port	 -- port on which to start the server
void servehttp(unsigned short port, char* files) {

	assert(files!=NULL);

	app_t app;
	appargs_t args;
	args.files = files;
	app.run = &appfunc;
	app.args = &args;
	servetcp(port, &app);
}

/***************************************************************************
 *    Private functions
 */

// Appends date and server name headers to the dynamic string
void append_date_and_server_headers(dynstr_t* s) {

	assert(s!=NULL);

	str_onto_dynstr(s, DATE_HEADER);
	str_onto_dynstr(s, SERVER_HEADER);
}

// Appends Content-Length header to the dynamic string
void append_contentlen_header(dynstr_t* s, int contentlen) {

	assert(s!=NULL);

	char header[strlen(CONTENT_LEN_HEADER) + INT_DIGITS + 1];
	sprintf(header, CONTENT_LEN_HEADER, contentlen);
	str_onto_dynstr(s, header);
}

// Appends Content-Type header to the dynamic string
void append_mimetype_header(dynstr_t* s, char* filename) {

	assert(filename != NULL);
	assert(s!=NULL);
	
	int i = 0;
	int n = strlen(filename);
	char* extension = NULL;
	char header[strlen(MIMETYPE_HEADER) + MIMETYPE_MAXLEN + 1];

	// get extension
	for (i=n; i>=0; i--) {
		if (filename[i]==DOT) {
			extension = filename + i;
			break;
		}
	}

	// Map extension to MIMETYPE
	if (!strcmp(extension, MIMETYPE_JPG_X)
		|| !strcmp(extension, MIMETYPE_JPEG_X)) {
		sprintf(header, MIMETYPE_HEADER, MIMETYPE_JPEG);

	} else if (!strcmp(extension, MIMETYPE_HTML_X)) {
		sprintf(header, MIMETYPE_HEADER, MIMETYPE_HTML);

	} else if (!strcmp(extension, MIMETYPE_CSS_X)) {
		sprintf(header, MIMETYPE_HEADER, MIMETYPE_CSS);

	} else if (!strcmp(extension, MIMETYPE_JS_X)) {
		sprintf(header, MIMETYPE_HEADER, MIMETYPE_JS);

	} else  if (!strcmp(extension, MIMETYPE_PLAIN_X)) {
		sprintf(header, MIMETYPE_HEADER, MIMETYPE_PLAIN);

	} else {
		sprintf(header, MIMETYPE_HEADER, MIMETYPE_PLAIN);
	}
	
	str_onto_dynstr(s, header);
}

// Appends Content-Type header to the dynamic string
void append_statusline(dynstr_t* s, char* status_str) {

	assert(s!=NULL);
	assert(status_str!=NULL);

	str_onto_dynstr(s, HTTP_VERSION);
	str_onto_dynstr(s, SPACE);
	str_onto_dynstr(s, status_str);
	str_onto_dynstr(s, CRLF);
}

// Maps http error codes to messages
char* http_err_to_msg(int errcode) {

	char* error_msg_print = NULL;

	if (errcode == STATUS_501) {
		error_msg_print = STATUS_501_PRINT;
	} else if (errcode == STATUS_404) {
		error_msg_print = STATUS_404_PRINT;
	} else if (errcode == STATUS_414) {
		error_msg_print = STATUS_414_PRINT;
	} else {
		error_msg_print = STATUS_400_PRINT;
	}
	return error_msg_print;
}

// Send an error response
void http_err(int fd, int errcode) {

	dynstr_t* response = new_dynstr();
	dynstr_t* content = new_dynstr();
	char* error_msg_print = http_err_to_msg(errcode);

	// Compose response
	append_statusline(response, error_msg_print);
	str_onto_dynstr(content, error_msg_print);
	append_date_and_server_headers(response);
	append_contentlen_header(response, content->len);
	append_mimetype_header(response, MIMETYPE_HTML_X);
	str_onto_dynstr(response, CRLF);
	dynstr_onto_dynstr(response, content);

	// Write response to fd
	write(fd, response->s, response->len);
	//printf("===ERROR: %d===\n%s\n", errcode, response->s);

	// Cleanup
	free_dynstr(response);
	free_dynstr(content);
}

// Sends a file over http
void send_file(int fd, char* filename) {

	assert(filename != NULL);

	dynstr_t* content = NULL;
	dynstr_t* response = NULL;
	FILE* fp = NULL;

	// Open & load the file
	fp = fopen(filename, READ_BINARY);
	if (fp == NULL) {
		http_err(fd, STATUS_404);
		return;
	}

	// Compose http response
	response = new_dynstr();
	content = file_into_dynstr(fp);
	append_statusline(response, STATUS_200_PRINT);
	append_date_and_server_headers(response);
	append_contentlen_header(response, content->len);
	append_mimetype_header(response, filename);
	str_onto_dynstr(response, CRLF);
	dynstr_onto_dynstr(response, content);

	// Send
	write(fd, response->s, response->len);
	//printf("===SUCCESS===\n%s\n", response->s);

	// Cleanup
	free_dynstr(response);
	free_dynstr(content);
	fclose(fp);
}

// Reads the request line into method and url buffer arrays
int read_request_line(int fd, char* methodbuf, char* urlbuf) {
	
	assert(methodbuf!=NULL);
	assert(urlbuf!=NULL);

	dynstr_t* requestline = NULL;
	int parsed = 0;

	requestline = readline_to_dynstr(fd);
	if (requestline == NULL) { return parsed; }
	printf("Recieved request: %s\n", requestline->s);
	parsed = sscanf(requestline->s,"%s %s", methodbuf, urlbuf);
	free_dynstr(requestline);
	return parsed;
}

// Serves files to a file descriptor over http
// fd -- File descripter to read/write
void appfunc(int fd, void* args) {

	assert(args!=NULL);

	appargs_t* appargs = (appargs_t*)args;
	char method[METHOD_LEN];
	char url[URL_LEN];

	// Parse request line
	memset(method, NULLBYTE, METHOD_LEN);
	memset(url, NULLBYTE, URL_LEN);
	int parsed = read_request_line(fd, method, url);

	// Choose a response
	short urltoolong = url[URL_LEN-1] != NULLBYTE;
	short parse_failed = parsed!=REQINE_PARSE;
	short unsupported_method = strcmp(method, METHOD_GET) != 0;

	if (urltoolong) {
		http_err(fd, STATUS_414);
	} else if (parse_failed) {
		http_err(fd, STATUS_400);
	} else if (unsupported_method) {
		http_err(fd, STATUS_501);
	} else {
		// make a filename string
		char filename[strlen(appargs->files)+strlen(url)+1];
		strcpy(filename, appargs->files);
		strcpy(filename+strlen(filename), url);

		// send file
		send_file(fd, filename);
	}
}
