/*

  wthnewpd.c

  server to read WS2000 weatherstation and pcwsr weathersensor receiver
  
  threaded version
  each thread handles the serial data read,
  right now data are echoed to standard out

  Copyright (C) 2002,2007 Volker Jahns, Volker.Jahns@thalreit.de

*/
#include "wthnew.h"


int
main ( int argc, char **argv ) 
{
  int ret;
  pthread_t ptid, wtid;

  char *ws2000lck = WS2000LOCK;

  initdata();
  openlog("wthd", LOG_PID , wsconf.logfacility);
  syslog(LOG_INFO, "wthd: %s begin of execution\n", VERSION);
  unlink( ws2000lck);
  tzset(); /* setting timezone */

  /* PCWSR thread */
  if ( strncmp( pcwsrstation.config.device, "/dev/", 5) == 0) {
    if ( ( ret = pthread_create( &ptid, NULL, ploghandler, NULL) == 0)) {
	syslog(LOG_INFO, "wthd: creating PCWSR thread: success\n");
    } else {
	syslog(LOG_ALERT,"wthd: error! Can't create PCWSR thread\n");
    }
    syslog(LOG_DEBUG,"wthd: pcwsrstation.config.device: %s\n", 
      pcwsrstation.config.device);
  }

  /* WS2000 thread */
  if ( strncmp( ws2000station.config.device, "/dev/", 5) == 0) {
    if ( ( ret = pthread_create( &wtid, NULL, wloghandler, NULL) == 0)) {
      syslog(LOG_DEBUG, "wthd: creating WS2000 thread: success");
    } else {
      syslog(LOG_ALERT,"wthd: error! Can't create WS2000 thread");
    } 
    syslog(LOG_DEBUG, "wthd: ws2000station.config.device: %s\n", 
      ws2000station.config.device);
  }
  /* handling interactive commands */
  //pthread_create( &ctid, NULL, cmdhandler, NULL);
  //pthread_join( &tid_cmdhandler, NULL);

  if ( strncmp( pcwsrstation.config.device, "/dev/", 5) == 0) {
    pthread_join( ptid, NULL);
  }
  if ( strncmp( ws2000station.config.device, "/dev/", 5) == 0) {
    pthread_join( wtid, NULL);
  }
  exit(0);
}
