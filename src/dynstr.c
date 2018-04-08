/***************************************************************************
 *
 *   File        : dynstr.c
 *   Student Id  : 757931
 *   Name        : Sebastian Baker
 *
 ***************************************************************************/

#include "dynstr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define DEFAULT_SIZE 8
#define FSEEK_START 0

// Creates a new empty dynamic string
dynstr_t* new_dynstr() {

	int size = DEFAULT_SIZE;
	dynstr_t* s = (dynstr_t*)malloc(sizeof(dynstr_t));
	assert(s!=NULL);

	s->s = (char*)malloc(size * sizeof(char));
	assert(s->s!=NULL);
	memset(s->s, NULLBYTE, size);

	s->len = 0;
	s->size = size;
	return s;
}

// Creates a dynstr and reads a file into it
dynstr_t* file_into_dynstr(FILE* fp) {

	assert(fp!=NULL);
	
	int newsize = 0;
	dynstr_t* s = new_dynstr();

	fseek(fp, FSEEK_START, SEEK_END);
	newsize = ftell(fp);
	rewind(fp);

	resize_dynstr(s, newsize);
	fread(s->s, s->size, sizeof(char), fp);
	s->len = s->size;
	return s;
}

// Reads a single line from fd
dynstr_t* readline_to_dynstr(int fd) {
	
	int n = 0;
	char c = NULLBYTE;
	dynstr_t* line = new_dynstr();

	// Skip past any initial newlines, carriage returns or nullbytes
	do {
		n = read(fd, &c, sizeof(char));
	} while (n && (c==LF || c==CR || c==NULLBYTE));

	// Load useful characters into the dynamic string
	do {
		char_onto_dynstr(line, c);
		n = read(fd, &c, sizeof(char));
	} while (n && c!=LF && c!=CR);

	// Return NULL if conn broke before line could be finished
	if (!n) {
		free_dynstr(line);
		return NULL;
	}

	return line;
}

// Resizes a dynstr
void resize_dynstr(dynstr_t* s, unsigned int newsize) {
	
	assert(s!=NULL);

	char* tip = NULL;
	s->size = newsize;
	s->s = (char*)realloc(s->s, s->size);
	assert(s->s!=NULL);

	tip = s->s + s->len;
	memset(tip, NULLBYTE, s->size-s->len);
}

// Free memory for a dynstr
void free_dynstr(dynstr_t* s) {
	assert(s!=NULL);
	free(s->s);
	free(s);
}

// Appends a string onto the end of a dynstr
void str_onto_dynstr(dynstr_t* s, char* str) {

	assert(s!=NULL);
	assert(str!=NULL);

	char* tip = NULL;

	// Extend memory allocation
	s->size += strlen(str);
	s->s = (char*)realloc(s->s, s->size);
	assert(s->s!=NULL);
	tip = s->s + s->len;
	memset(tip, NULLBYTE, s->size-s->len);

	// copy new str into dynstr
	strcpy(tip, str);
	s->len += strlen(str);
}

// Appends a dystr onto the end of a dynstr
void dynstr_onto_dynstr(dynstr_t* s, dynstr_t* str) {

	assert(s!=NULL);
	assert(str!=NULL);

	char* tip = NULL;

	// Extend memory allocation
	s->size += str->len;
	s->s = (char*)realloc(s->s, s->size);
	assert(s->s!=NULL);

	// Zero new memory area
	tip = s->s + s->len;
	memset(tip, NULLBYTE, s->size-s->len);

	// copy new str into dynstr
	memcpy((void*)tip, (void*)str->s, str->len);
	s->len += str->len;
}

// Append a char onto a dynstr
void char_onto_dynstr(dynstr_t* s, char c) {

	assert(s!=NULL);
	
	// Reallocate memory
	s->size++;
	s->s = (char*)realloc(s->s, s->size);
	assert(s->s!=NULL);

	// Append new character
	memset(s->s+s->len, NULLBYTE, s->size-s->len);
	s->s[s->len] = c;
	s->len++;
}
