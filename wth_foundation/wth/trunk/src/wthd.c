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
  pthread_t ptid, wtid, owtid, wmrtid, umtid;

  nobg = 0;
  /* parse commandline */
  while ((op = getopt(argc, argv, "dp:f:h")) != -1) {
    switch(op) {
    case 'd':
      nobg = 1;
      break;
    case 'f':
      wsconf.configfile = strdup(optarg);
      break;
    case 'p':
      wsconf.port = strdup(optarg);
      break;
    case '?':
      usaged(1,"command line error","");
      break;
    case 'h':
      usaged(0,"","");
      break;
    default:
      usaged(0, "", "");
    }
  }

  /* reading config file and default parameters */
  if  (( ret = wthd_init()) != 0 ) {
    printf("wthd: can't initialize. Exit!\n");
    exit(ret);
  }


  if ( nobg == 0 ) {
    //daemon_init();
    openlog("wthd", LOG_PID , wsconf.log_facility);
  } else {
    openlog("wthd", LOG_PID | LOG_PERROR , wsconf.log_facility);
  }
  syslog(LOG_INFO, "wthd: %s begin of execution\n", VERSION);

  tzset();

  /* PCWSR thread */
  if ( strncmp( pcwsrstation.config.device, "/dev/", 5) == 0) {
    syslog(LOG_INFO,"wthd: PCWSR weatherstation configured");
    syslog(LOG_DEBUG,"wthd: pcwsrstation.config.device: %s", 
      pcwsrstation.config.device);
    if ( ( ret = pthread_create( &ptid, NULL, pcwsr_hd, NULL) == 0)) {
        pcwsrstation.status.is_present = 1;
	syslog(LOG_INFO, "wthd: creating PCWSR thread: success\n");
    } else {
        pcwsrstation.status.is_present = -1;
	syslog(LOG_ALERT,"wthd: error! Can't create PCWSR thread\n");
    }
  } else { syslog(LOG_INFO, "wthd: no PCWSR serialport configured\n"); }

  /* WS2000 thread */
  if ( strncmp( ws2000station.config.device, "/dev/", 5) == 0) {
    syslog(LOG_INFO,"wthd: WS2000 weatherstation configured");
    syslog(LOG_DEBUG, "wthd: ws2000station.config.device: %s\n", 
      ws2000station.config.device);
    if ( ( ret = pthread_create( &wtid, NULL, ws2000_hd, NULL) == 0)) {
      ws2000station.status.is_present = 1;
      syslog(LOG_DEBUG, "wthd: creating WS2000 thread: success");
    } else {
      ws2000station.status.is_present = -1;
      syslog(LOG_ALERT,"wthd: error! Can't create WS2000 thread");
    } 
  } else { syslog(LOG_INFO, "wthd: no WS2000 serialport configured\n"); } 


  /* 1-Wire thread */
  if ( strncmp( onewirestation.config.device, "/dev/", 5) == 0) {
    syslog(LOG_INFO,"wthd: 1-Wire weatherstation configured");
    syslog(LOG_DEBUG, "wthd: onewirestation.config.device: %s\n", 
      onewirestation.config.device);
    if ( ( ret = pthread_create( &owtid, NULL, onewire_hd, NULL) == 0)) {
      onewirestation.status.is_present = 1;
      syslog(LOG_DEBUG, "wthd: creating 1-Wire thread: success");
    } else {
      onewirestation.status.is_present = -1;
      syslog(LOG_ALERT,"wthd: error! Can't create 1-Wire thread");
    } 
  } else { syslog(LOG_INFO, "wthd: no 1-Wire port configured\n"); }

  /* WMR9x8 thread */
  if ( strncmp( wmr9x8station.config.device, "/dev/", 5) == 0) {
    syslog(LOG_INFO,"wthd: WMR9x8 weatherstation configured");
    syslog(LOG_DEBUG, "wthd: wmr9x8.config.device: %s\n", 
      wmr9x8station.config.device);
    if ( ( ret = pthread_create( &wmrtid, NULL, wmr9x8_hd, NULL) == 0)) {
      wmr9x8station.status.is_present = 1;
      syslog(LOG_DEBUG, "wthd: creating wmr9x8 thread: success");
    } else {
      wmr9x8station.status.is_present = -1;
      syslog(LOG_ALERT,"wthd: error! Can't create wmr9x8 thread");
    } 
  } else { syslog(LOG_INFO,"wthd: no WMR9x8 port configured\n"); }

  /* ULTIMETER thread */
  if ( strncmp( umeterstation.config.device, "/dev/", 5) == 0) {
    syslog(LOG_INFO,"wthd: ULTIMETER weatherstation configured");
    syslog(LOG_DEBUG, "wthd: umeterstation.config.device: %s\n", 
      umeterstation.config.device);
    if ( ( ret = pthread_create( &umtid, NULL, umeter_hd, NULL) == 0)) {
      umeterstation.status.is_present = 1;
      syslog(LOG_DEBUG, "wthd: creating umeter thread: success");
    } else {
      umeterstation.status.is_present = -1;
      syslog(LOG_ALERT,"wthd: error! Can't create ULTIMETER thread");
    } 
  } else { syslog(LOG_INFO,"wthd: no ULTIMETER port configured\n"); }


  for(;;) { sleep(120); continue;}
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
