/***************************************************************************
 *
 *   File        : tcpserver.h
 *   Student Id  : 757931
 *   Name        : Sebastian Baker
 *
 ***************************************************************************/

#ifndef TCPSERVER_H

typedef void handlerfunc_t(unsigned int fd);

int tcpserve(unsigned short port, handlerfunc_t* handler);

#endif
