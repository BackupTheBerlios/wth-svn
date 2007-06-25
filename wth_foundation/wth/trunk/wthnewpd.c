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
  pthread_t tid_pcwsr,
    tid_ws2000,
    tid_cmdhandle;

  char *ws2000lck = WS2000LOCK;

  initdata();
  openlog("wthnewpd", LOG_PID , wsconf.logfacility);
  syslog(LOG_INFO, "wthnewd: %s\n", VERSION);
  unlink( ws2000lck);
  tzset(); /* setting timezone */

  /* PCWSR thread */
  //printf("Creating PCWSR thread: ");
  //ret = pthread_create( &tid_pcwsr, NULL, pcwsr_loghandler, NULL);
  //printf ("ret : %d\n", ret);

  /* WS2000 thread */
  printf("Creating WS2000 thread: ");
  ret = pthread_create( &tid_ws2000, NULL, ws2000_loghandler, NULL);
  printf ("ret : %d\n", ret);

  /* handling interactive commands */
  //pthread_create( &tid_cmdhandler, NULL, cmdhandler, NULL);

  //pthread_join( &tid_cmdhandler, NULL);
  pthread_join( tid_ws2000, NULL);
  //pthread_join( tid_pcwsr, NULL);

  exit(0);
}

