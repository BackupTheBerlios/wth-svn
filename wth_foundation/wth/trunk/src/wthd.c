/*

  wthd.c

  server to read 
  - WS2000 weatherstation 
  - pcwsr weathersensor receiver
  
  threaded version
  each thread handles the serial data read,
  right now data are echoed to standard out

  $Id: wthd.c 207 2008-10-17 18:43:08Z vjahns $
  $Revision: 207 $

  Copyright (C) 2002-2008 Volker Jahns, volker@thalreit.de

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 
*/
#include "wth.h"

int
main ( int argc, char **argv ) 
{
  int op, ret, nobg;
  pthread_t ctid, ptid, wtid, owtid, stoptid;
  sigset_t signals_to_block;

  char *lockfile = WS2000LOCK;

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
  if ( strncmp( ws2000station.config.device, "/dev/", 5) == 0) {
    if ( ( ret = pthread_create( &wtid, NULL, ws2000_hd, NULL) == 0)) {
      syslog(LOG_DEBUG, "wthd: creating WS2000 thread: success");
    } else {
      syslog(LOG_ALERT,"wthd: error! Can't create WS2000 thread");
    } 
    syslog(LOG_DEBUG, "wthd: ws2000station.config.device: %s\n", 
      ws2000station.config.device);
  } else { printf("wthd: no WS2000 serialport configured\n"); }


  /* 1-Wire thread */
  printf("wthd: onewirestation.config.device: \"%s\"\n", 
    onewirestation.config.device);
  if ( strncmp( onewirestation.config.device, "/dev/", 5) == 0) {
    if ( ( ret = pthread_create( &wtid, NULL, ws2000_hd, NULL) == 0)) {
      syslog(LOG_DEBUG, "wthd: creating 1-Wire thread: success");
    } else {
      syslog(LOG_ALERT,"wthd: error! Can't create 1-Wire thread");
    } 
    syslog(LOG_DEBUG, "wthd: onewirestation.config.device: %s\n", 
      onewirestation.config.device);
  } else { printf("wthd: no 1-Wire port configured\n"); }


  /* thread to handle interactive commands */
  pthread_create( &ctid, NULL, cmd_hd, NULL);
  pthread_join( ctid, NULL);

  if ( strncmp( pcwsrstation.config.device, "/dev/", 5) == 0) {
    pthread_join( ptid, NULL);
  }
  if ( strncmp( ws2000station.config.device, "/dev/", 5) == 0) {
    pthread_join( wtid, NULL);
  }
  if ( strncmp( onewirestation.config.device, "/dev/", 5) == 0) {
    pthread_join( owtid, NULL);
  }
  exit(0);
}
