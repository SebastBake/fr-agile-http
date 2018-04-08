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

typedef struct {
	appfunc_t* run; // Function to run the application
	void* args; // arguments for the application
} app_t;

void servetcp(unsigned short port, app_t* app);

#endif
