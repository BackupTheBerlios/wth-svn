/*

  wthnewpd.c

  server to read WS2000 weatherstation and pcwsr weathersensor receiver
  
  threaded version
  each thread handles the serial data read,
  right now data are echoed to standard out

  Copyright (C) Jun 2002,2007 Volker Jahns, Volker.Jahns@thalreit.de

*/
#include "wthnew.h"


int
main ( int argc, char **argv ) 
{
  int ret;
  pthread_t ptid, wtid, ctid;

  char *ws2000lck = WS2000LOCK;

  initdata();
  openlog("wthnewpd", LOG_PID , wsconf.logfacility);
  syslog(LOG_INFO, "wthnewd: %s\n", VERSION);
  unlink( ws2000lck);
  tzset(); /* setting timezone */

  /* PCWSR thread */
  printf("Creating PCWSR thread: ");
  ret = pthread_create( &ptid, NULL, ploghandler, NULL);
  printf ("ret : %d\n", ret);

  /* WS2000 thread */
  printf("Creating WS2000 thread: ");
  ret = pthread_create( &wtid, NULL, wloghandler, NULL);
  printf ("ret : %d\n", ret);

  /* handling interactive commands */
  //pthread_create( &ctid, NULL, cmdhandler, NULL);

  //pthread_join( &tid_cmdhandler, NULL);
  pthread_join( wtid, NULL);
  pthread_join( ptid, NULL);

  exit(0);
}

