/*

  wthd.c

  $Id: wthd.c 207 2008-10-17 18:43:08Z vjahns $
  $Revision: 207 $

  server to read 
  - WS2000 weatherstation 
  - pcwsr weathersensor receiver
  
  threaded version
  each thread handles the serial data read,
  right now data are echoed to standard out

  Copyright (C) 2002-2008 Volker Jahns, volker@thalreit.de

*/
#include "wth.h"


int
main ( int argc, char **argv ) 
{
  int op, ret, nobg;
  pthread_t ctid, ptid, wtid, stoptid;
  sigset_t signals_to_block;

  char *lockfile = WS2000LOCK;
  //char *ws2000lck = WS2000LOCK;

  nobg = 0;
  /* parse commandline */
  while ((op = getopt(argc, argv, "dp:")) != -1) {
    switch(op) {
    case 'd':
      nobg = 1;
      break;
    case 'p':
      wsconf.port = strdup(optarg);
      break;
    case '?':
      usaged(1,"command line error","");
      break;
    default:
      usaged(0, "", "");
    }
  }

  /* reading config file and default parameters */
  if  (( ret = wthd_init()) != 0 ) {
    printf("wthd: can't initialize. Exit!\n");
  }


  if ( nobg == 0 ) {
    daemon_init();
    openlog("wthd", LOG_PID , wsconf.logfacility);
  } else {
    openlog("wthd", LOG_PID | LOG_PERROR , wsconf.logfacility);
  }
  syslog(LOG_INFO, "wthd: %s begin of execution\n", VERSION);

  //unlink( ws2000lck);
  unlink( lockfile);
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
  printf("wthd: ws2000station.config.device: \"%s\"\n", 
    ws2000station.config.device);
  if ( strncmp( ws2000station.config.device, "/dev/", 5) == 0) {
    if ( ( ret = pthread_create( &wtid, NULL, ws2000_hd, NULL) == 0)) {
      syslog(LOG_DEBUG, "wthd: creating WS2000 thread: success");
    } else {
      syslog(LOG_ALERT,"wthd: error! Can't create WS2000 thread");
    } 
    syslog(LOG_DEBUG, "wthd: ws2000station.config.device: %s\n", 
      ws2000station.config.device);
  } else { printf("wthd: no device configured\n"); }

  /* thread to handle interactive commands */
  pthread_create( &ctid, NULL, cmd_hd, NULL);
  pthread_join( ctid, NULL);

  if ( strncmp( pcwsrstation.config.device, "/dev/", 5) == 0) {
    pthread_join( ptid, NULL);
  }
  if ( strncmp( ws2000station.config.device, "/dev/", 5) == 0) {
    pthread_join( wtid, NULL);
  }
  exit(0);
}
