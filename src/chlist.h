/***************************************************************************
 *
 *   File        : chlist.h
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

typedef struct {
	char* s;
	unsigned int size;
	unsigned int len;
} chlist_t;

chlist_t* new_chlist();
chlist_t* file_into_chlist(FILE* fp);
chlist_t* readline_to_chlist(int fd);
void free_chlist(chlist_t* s);
void resize_chlist(chlist_t* s, unsigned int size);
void str_onto_chlist(chlist_t* s, char* str);
void chlist_onto_chlist(chlist_t* s, chlist_t* str);
void char_onto_chlist(chlist_t* s, char c);

#endif