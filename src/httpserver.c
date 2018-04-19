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
#include <time.h>
#include "chlist.h"
#include "httpserver.h"
#include "tcpserver.h"

#define DEBUG 0

#define WRITE_ERR -1
#define DOT '.'
#define SPACE " "
#define CRLF "\r\n"
#define SEND_MSG "SENT %d bytes: %s\n"
#define SEND_FAIL_MSG "SEND FAIL (%d+%d of %d bytes)\n"
#define SYSYEAR_OFFSET 1900

#define CONTENT_LEN_HEADER "Content-Length: %d\r\n"
#define MIMETYPE_HEADER "Content-Type: %s\r\n"
#define SERVER_HEADER "Server: Hal9000\r\n"
#define DATE_HEADER "Date: %d:%d:%d:%d:%d:%d GMT\r\n"
#define MIMETYPE_HEADER_MAXLEN 256
#define CONTENT_LEN_HEADER_MAXLEN 256
#define DATE_HEADER_MAXLEN 256

#define METHOD_GET "GET"
#define HTTP_VERSION "HTTP/1.1"
#define REQ_TEMPLATE "%s %s %*s"
#define REQ_PARSE 2
#define REQ_MSG "Recieved request: %s\n"
#define HTTP_VERSION_LEN 16
#define METHOD_LEN 8
#define URL_LEN 256

#define STATUS_200 "200 OK"
#define STATUS_400 "400 Bad Request"
#define STATUS_404 "404 Not Found"
#define STATUS_414 "414 URI Too Long"
#define STATUS_501 "501 Not Implemented"

#define NUM_MIMETYPES 6
#define MIMETYPE_STRUCT struct {char* EXT; char* TYPE; }
const MIMETYPE_STRUCT MIMETYPES[NUM_MIMETYPES] = {
	{.EXT=".jpeg", "image/jpeg"},
	{.EXT=".jpg", "image/jpeg"},
	{.EXT=".html", "text/html"},
	{.EXT=".txt", "text/plain"},
	{.EXT=".js", "application/javascript"},
	{.EXT=".css", "text/css"}
};

void append_server_header(chlist_t* s);
void append_date_header(chlist_t* s);
void append_contentlen_header(chlist_t* s, int contentlen);
void append_mimetype_header(chlist_t* s, char* filename);
void append_statusline(chlist_t* s, char* status_str);

void http_err(int fd, char* errcode);
void send_file(int fd, char* filename);
void httpapp(int fd, void* args);
void writeall(int fd, chlist_t* chlist);

int read_request_line(int fd, char* methodbuf, char* urlbuf);
char* http_err_to_msg(int errcode);

void debugprint(char* msg);


/***************************************************************************
 *    Public functions
 */


// Serves files over http
// files -- root folder from which to serve files
// port	 -- port on which to start the server
void servehttp(unsigned short port, char* files) {
	assert(files!=NULL);
	servetcp(port, &httpapp, files);
}


/***************************************************************************
 *    Private functions
 */


// Appends date and server name headers to the dynamic string
void append_date_header(chlist_t* s) {

	assert(s!=NULL);

	time_t unixtime;
	time(&unixtime);
	char header[DATE_HEADER_MAXLEN];
	struct tm *now = gmtime(&unixtime);
	assert(now!=NULL);

    sprintf(header, DATE_HEADER,
    	now->tm_year + SYSYEAR_OFFSET,
    	now->tm_mon,
    	now->tm_mday,
    	now->tm_hour,
    	now->tm_min,
    	now->tm_sec
    	);
	str_onto_chlist(s, header);
}


// Appends server name headers to the dynamic string
void append_server_header(chlist_t* s) {
	assert(s!=NULL);
	str_onto_chlist(s, SERVER_HEADER);
}


// Appends Content-Length header to the dynamic string
void append_contentlen_header(chlist_t* s, int contentlen) {

	assert(s!=NULL);

	char header[CONTENT_LEN_HEADER_MAXLEN];
	sprintf(header, CONTENT_LEN_HEADER, contentlen);
	str_onto_chlist(s, header);
}


// Appends Content-Type header to the dynamic string
// If extension is unknown, does nothing
void append_mimetype_header(chlist_t* s, char* filename) {

	assert(filename != NULL);
	assert(s!=NULL);
	
	int i = 0;
	int n = strlen(filename);
	char* extension = NULL;
	char header[MIMETYPE_HEADER_MAXLEN];

	// get extension
	for (i=n; i>=0; i--) {
		if (filename[i]==DOT) {
			extension = filename + i;
			break;
		}
	}

	// Map extension to MIMETYPE
	for (i=0; i<NUM_MIMETYPES; i++) {
		if (!strcmp(extension, MIMETYPES[i].EXT)) {
			sprintf(header, MIMETYPE_HEADER, MIMETYPES[i].TYPE);
			str_onto_chlist(s, header);
		}
	}
}


// Appends Content-Type header to the dynamic string
void append_statusline(chlist_t* s, char* status_str) {

	assert(s!=NULL);
	assert(status_str!=NULL);

	str_onto_chlist(s, HTTP_VERSION);
	str_onto_chlist(s, SPACE);
	str_onto_chlist(s, status_str);
	str_onto_chlist(s, CRLF);
}


// Send an error response
void http_err(int fd, char* errcode) {

	chlist_t* response = new_chlist();
	chlist_t* content = new_chlist();

	// Compose response
	str_onto_chlist(content, errcode);
	append_statusline(response, errcode);
	append_date_header(response);
	append_server_header(response);
	append_contentlen_header(response, content->len);
	str_onto_chlist(response, CRLF);

	// Write response to fd
	writeall(fd, response);
	writeall(fd, content);

	// Cleanup
	free_chlist(response);
	free_chlist(content);
}


// Sends a file over http
void send_file(int fd, char* filename) {

	assert(filename != NULL);

	chlist_t* content = NULL;
	chlist_t* response = NULL;

	// Open & load the file
	content = file_into_chlist(filename);
	if (content == NULL) {
		http_err(fd, STATUS_404);
		return;
	}

	// Compose http response
	response = new_chlist();
	append_statusline(response, STATUS_200);
	append_date_header(response);
	append_server_header(response);
	append_contentlen_header(response, content->len);
	append_mimetype_header(response, filename);
	str_onto_chlist(response, CRLF);

	// Sends
	writeall(fd, response);
	writeall(fd, content);

	// Cleanup
	free_chlist(response);
	free_chlist(content);
}


// Reads the request line into method and url buffer arrays
int read_request_line(int fd, char* methodbuf, char* urlbuf) {
	
	assert(methodbuf!=NULL);
	assert(urlbuf!=NULL);

	chlist_t* requestline = NULL;
	int parsed = 0;

	requestline = readline_to_chlist(fd);
	if (requestline == NULL) { return parsed; }
	if(DEBUG) { printf(REQ_MSG, requestline->s); }
	parsed = sscanf(requestline->s, REQ_TEMPLATE, methodbuf, urlbuf);
	free_chlist(requestline);
	return parsed;
}


// Serves files to a file descriptor over http
// fd -- File descripter to read/write
void httpapp(int fd, void* args) {

	assert(args!=NULL);

	char* files = (char*)args;
	char method[METHOD_LEN];
	char url[URL_LEN];

	// Parse request line
	memset(method, NULLBYTE, METHOD_LEN);
	memset(url, NULLBYTE, URL_LEN);
	int parsed = read_request_line(fd, method, url);

	// Choose a response
	short urltoolong = url[URL_LEN-1] != NULLBYTE;
	short parse_failed = parsed!=REQ_PARSE;
	short unsupported_method = strcmp(method, METHOD_GET) != 0;

	if (urltoolong) {
		http_err(fd, STATUS_414);
	} else if (parse_failed) {
		http_err(fd, STATUS_400);
	} else if (unsupported_method) {
		http_err(fd, STATUS_501);
	} else {
		// make a filename string
		char filename[strlen(files)+strlen(url)+1];
		strcpy(filename, files);
		strcpy(filename+strlen(filename), url);

		// send file
		send_file(fd, filename);
	}
}


void writeall(int fd, chlist_t* chlist) {

	assert(chlist!=NULL);

	int wrote = 0, wrotesum = 0;

	while (wrotesum < chlist->len) {
		wrote = write(fd, chlist->s+wrotesum, chlist->len-wrotesum);
		if (wrote == WRITE_ERR || 0) {  // Give up writing to socket
			if (DEBUG) {
				printf(SEND_FAIL_MSG, wrotesum, wrote, chlist->len);
			}
			return;
		}
		wrotesum +=wrote;
	}
	if (DEBUG) {
		printf(SEND_MSG, wrotesum, chlist->s);
	}
}
