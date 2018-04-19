/***************************************************************************
 *
 *   File        : tcpserver.h
 *   Student Id  : 757931
 *   Name        : Sebastian Baker
 *
 ***************************************************************************/

#ifndef __TCPSERVER_H__

/* The app uses the tcp stream to do something useful, apps must be of type
 * appfunc_t so that this tcp library can run the app */
typedef void appfunc_t(int fd, void* args);

void servetcp(unsigned short port, appfunc_t* appfunc, void* appargs);

#endif
