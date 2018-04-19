/***************************************************************************
 *
 *   File        : chlist.c
 *   Student Id  : 757931
 *   Name        : Sebastian Baker
 *
 ***************************************************************************/

#include "chlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define DEFAULT_SIZE 8
#define FSEEK_START 0

// Creates a new empty dynamic string
chlist_t* new_chlist() {

	int size = DEFAULT_SIZE;
	chlist_t* s = (chlist_t*)malloc(sizeof(chlist_t));
	assert(s!=NULL);

	s->s = (char*)malloc(size * sizeof(char));
	assert(s->s!=NULL);
	memset(s->s, NULLBYTE, size);

	s->len = 0;
	s->size = size;
	return s;
}

// Creates a chlist and reads a file into it
chlist_t* file_into_chlist(FILE* fp) {

	assert(fp!=NULL);
	
	int newsize = 0;
	chlist_t* s = new_chlist();

	fseek(fp, FSEEK_START, SEEK_END);
	newsize = ftell(fp);
	rewind(fp);

	resize_chlist(s, newsize);
	fread(s->s, s->size, sizeof(char), fp);
	s->len = s->size;
	return s;
}

// Reads a single line from fd
chlist_t* readline_to_chlist(int fd) {
	
	int n = 0;
	char c = NULLBYTE;
	chlist_t* line = new_chlist();

	// Skip past any initial newlines, carriage returns or nullbytes
	do {
		n = read(fd, &c, sizeof(char));
	} while (n && (c==LF || c==CR || c==NULLBYTE));

	// Load useful characters into the dynamic string
	do {
		char_onto_chlist(line, c);
		n = read(fd, &c, sizeof(char));
	} while (n && c!=LF && c!=CR);

	// Return NULL if conn broke before line could be finished
	if (!n) {
		free_chlist(line);
		return NULL;
	}

	return line;
}

// Resizes a chlist
void resize_chlist(chlist_t* s, unsigned int newsize) {
	
	assert(s!=NULL);

	char* tip = NULL;
	s->size = newsize;
	s->s = (char*)realloc(s->s, s->size);
	assert(s->s!=NULL);

	tip = s->s + s->len;
	memset(tip, NULLBYTE, s->size-s->len);
}

// Free memory for a chlist
void free_chlist(chlist_t* s) {
	assert(s!=NULL);
	free(s->s);
	free(s);
}

// Appends a string onto the end of a chlist
void str_onto_chlist(chlist_t* s, char* str) {

	assert(s!=NULL);
	assert(str!=NULL);

	char* tip = NULL;

	// Extend memory allocation
	s->size += strlen(str);
	s->s = (char*)realloc(s->s, s->size);
	assert(s->s!=NULL);
	tip = s->s + s->len;
	memset(tip, NULLBYTE, s->size-s->len);

	// copy new str into chlist
	strcpy(tip, str);
	s->len += strlen(str);
}

// Appends a dystr onto the end of a chlist
void chlist_onto_chlist(chlist_t* s, chlist_t* str) {

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

	// copy new str into chlist
	memcpy((void*)tip, (void*)str->s, str->len);
	s->len += str->len;
}

// Append a char onto a chlist
void char_onto_chlist(chlist_t* s, char c) {

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
