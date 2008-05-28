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
  pthread_t ptid, wtid, stoptid;
  sigset_t signals_to_block;

  char *ws2000lck = WS2000LOCK;

  if  (( ret = wthd_init()) != 0 ) {
    printf("wthd: can't initialize. Exit!\n");
  }
  openlog("wthd", LOG_PID , wsconf.logfacility);
  syslog(LOG_INFO, "wthd: %s begin of execution\n", VERSION);
  unlink( ws2000lck);
  tzset();

  /* block signals in main */
  sigemptyset( &signals_to_block);
  sigaddset(&signals_to_block, SIGUSR1);
  pthread_sigmask(SIG_BLOCK, &signals_to_block, NULL);

  /* thread to catch shutdown signal */
  pthread_create(&stoptid, NULL, signal_hd, NULL);
  pthread_info.num_active = 0;
  pthread_cond_init(&pthread_info.thread_exit_cv, NULL);
  pthread_mutex_init(&pthread_info.mutex, NULL);

  /* PCWSR thread */
  if ( strncmp( pcwsrstation.config.device, "/dev/", 5) == 0) {
    if ( ( ret = pthread_create( &ptid, NULL, pcwsr_hd, NULL) == 0)) {
	syslog(LOG_INFO, "wthd: creating PCWSR thread: success\n");
    } else {
	syslog(LOG_ALERT,"wthd: error! Can't create PCWSR thread\n");
    }
    syslog(LOG_DEBUG,"wthd: pcwsrstation.config.device: %s\n", 
      pcwsrstation.config.device);
  }

  /* WS2000 thread */
  if ( strncmp( ws2000station.config.device, "/dev/", 5) == 0) {
    if ( ( ret = pthread_create( &wtid, NULL, ws2000_hd, NULL) == 0)) {
      syslog(LOG_DEBUG, "wthd: creating WS2000 thread: success");
    } else {
      syslog(LOG_ALERT,"wthd: error! Can't create WS2000 thread");
    } 
    syslog(LOG_DEBUG, "wthd: ws2000station.config.device: %s\n", 
      ws2000station.config.device);
  }
  /* handling interactive commands */
  //pthread_create( &ctid, NULL, cmd_hd, NULL);
  //pthread_join( &tid_cmd_hd, NULL);

  if ( strncmp( pcwsrstation.config.device, "/dev/", 5) == 0) {
    pthread_join( ptid, NULL);
  }
  if ( strncmp( ws2000station.config.device, "/dev/", 5) == 0) {
    pthread_join( wtid, NULL);
  }
  exit(0);
}
