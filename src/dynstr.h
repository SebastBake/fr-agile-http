/***************************************************************************
 *
 *   File        : dynstr.h
 *   Student Id  : 757931
 *   Name        : Sebastian Baker
 *
 ***************************************************************************/

#ifndef __DYNSTR_H__

#include <stdio.h>
#include <stdlib.h>

#define NULLBYTE '\0'
#define LF '\n'
#define CR '\r'
#define CRLF "\r\n"

typedef struct {
	char* s;
	unsigned int size;
	unsigned int len;
} dynstr_t;

dynstr_t* new_dynstr();
dynstr_t* file_into_dynstr(FILE* fp);
dynstr_t* readline_to_dynstr(int fd);
void free_dynstr(dynstr_t* s);
void resize_dynstr(dynstr_t* s, unsigned int size);
void str_onto_dynstr(dynstr_t* s, char* str);
void dynstr_onto_dynstr(dynstr_t* s, dynstr_t* str);
void char_onto_dynstr(dynstr_t* s, char c);

#endif