/* util.c

   collection of utility functions

   $Id: util.c 213 2008-10-25 20:33:26Z vjahns $
   $Revision: 213 $

   Copyright 2007,2008 Volker Jahns <volker@thalreit.de>

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


/* initdata 

   set initial values of global datastructures

   ws2000station
   pcwsrstation
   onewirestation
   wsconf
 
*/
int 
wthd_init( ) {
  int err;

  /* default parameters valid for all stations */
  wsconf.timeout     = 30;
  wsconf.logfacility = LOG_LOCAL5;
  wsconf.verbose     = 1;
  wsconf.debug       = 1;
  wsconf.hostname    = "localhost";
  if ( wsconf.port == NULL ) {
    wsconf.port        = "2000";
  }
  wsconf.units       = "SI";
  wsconf.outfmt      = "old";

  /* WS2000 */  
  strncpy(ws2000station.config.dbfile, "ws2000.db", TBUFF);
  strncpy(ws2000station.config.device, "n.a.", TBUFF);
  strncpy(ws2000station.config.rrdpath, ".", TBUFF);
  strncpy(ws2000station.config.monitor,"Sensormonitor.rrd", TBUFF);;
  ws2000station.status.interval      = 300;  

  /* allocate sensornames WS2000 */
  strncpy(ws2000station.sensor[1].sensorname, "Sensor1", TBUFF);
  strncpy(ws2000station.sensor[2].sensorname, "Sensor2", TBUFF);
  strncpy(ws2000station.sensor[3].sensorname, "Sensor3", TBUFF);
  strncpy(ws2000station.sensor[4].sensorname, "Sensor4", TBUFF);
  strncpy(ws2000station.sensor[5].sensorname, "Sensor5", TBUFF);
  strncpy(ws2000station.sensor[6].sensorname, "Sensor6", TBUFF);
  strncpy(ws2000station.sensor[7].sensorname, "Sensor7", TBUFF);
  strncpy(ws2000station.sensor[8].sensorname, "Sensor8", TBUFF);
  strncpy(ws2000station.sensor[9].sensorname, "Rainsensor", TBUFF);
  strncpy(ws2000station.sensor[10].sensorname, "Windsensor", TBUFF);
  strncpy(ws2000station.sensor[11].sensorname, "Indoorsensor", TBUFF);

  /* PCWSR */
  strncpy(pcwsrstation.config.dbfile, "pcwsr.db", TBUFF);
  strncpy( pcwsrstation.config.device, "n.a.", TBUFF);

  /* 1-WIRE */
  strncpy(onewirestation.config.dbfile, "onewire.db", TBUFF);
  strncpy(onewirestation.config.device, "n.a.", TBUFF);
  onewirestation.config.mcycle = 10;

  /* WMR9x8 */
  strncpy(wmr9x8station.config.dbfile, "wmr9x8.db", TBUFF);
  strncpy(wmr9x8station.config.device, "n.a.", TBUFF);

  /* ULTIMETER */
  strncpy(umeterstation.config.dbfile, "umeter.db", TBUFF);
  strncpy(umeterstation.config.device, "n.a.", TBUFF);

  err = readconfig();
  //printf("%s", echoconfig( "onewirestation"));
  printf("wthd_init: readconfig done\n");

  return(err);
}


/* echodata
  
   print out raw data frame reveived from weather station 

*/
int
echodata(unsigned char *data, int mdat) {
    int i;
    char frame[TBUFF] = {'\0'};
    char sf[4];

    syslog(LOG_DEBUG, "echodata : length dataframe : %d\n",mdat);

    /* for better readability label each byte in frame */  
    for ( i = 0; i < mdat; i++ ) {
      snprintf(sf, 4, "%02d:",i);
      strncat(frame, sf, 3);
    }
    syslog(LOG_DEBUG, "echodata : %s\n", frame);    
    strcpy(frame, "");

    for ( i = 0; i < mdat; i++ ) {
      snprintf(sf, 4, "%02x:",data[i]);
      strncat(frame, sf, 4);
    }

    syslog(LOG_DEBUG, "echodata : %s\n", frame);   

    return(0);
} 

/* 
   getbits

   return bit field of length n at position p of unsigned char x 
   as unsigned char 
*/
unsigned char
getbits(unsigned char x, int p, int n) {
  return ( x>>(p+1-n)) & ~(~0 <<n);
}

/* 
   lownibble

   return low nibble of unsigned char x ( BCD)
*/
unsigned char
lownibble(unsigned char x) {
  return  x & ~0xf0;
}

/* 
   highnibble

   return high nibble of unsigned char x ( BCD)
*/
unsigned char
highnibble(unsigned char x) {
  return ( x>>4) & ~0xf0;
}

/*
  bitprint 

  utility function to print bits of 8-bit byte
*/
int 
bitprint ( int byte, char *s_reg ) {
  int x;
  char frame[TBUFF+1];
  char sbuf[2];
  struct timezone tz;
  struct timeval  tv;

  gettimeofday(&tv, &tz);
  snprintf(frame, TBUFF, "%5s | %lu.%lu : #", s_reg, 
	 (long unsigned int) tv.tv_sec, (long unsigned int) tv.tv_usec);
  for( x = 7; x > -1; x-- ) {
    snprintf( sbuf, 2, "%i", (byte & 1 << x ) > 0 ? 1 : 0 );
    strncat( frame, sbuf, 2);
  }
  strncat(frame,"b", 2);
  syslog(LOG_DEBUG, "%s", frame);

  return(0);
}

/*
  longprint

  utility function to print bits of 16-bit byte
*/
int 
longprint ( int byte, char *s_reg ) {
  int x;
  char frame[TBUFF+1];
  char sbuf[2];
  struct timezone tz;
  struct timeval  tv;

  gettimeofday(&tv, &tz);
  snprintf(frame, 24, "%5s | %lu.%lu : #", s_reg, 
	 (long unsigned int) tv.tv_sec, (long unsigned int) tv.tv_usec);
  for( x = 15; x > -1; x-- ) {
    snprintf(sbuf, 1, "%i", (byte & 1 << x ) > 0 ? 1 : 0 );
    strncat(frame, sbuf, 2);
  }
  strncat(frame,"b", 2);
  syslog( LOG_DEBUG, "%s", frame);
  return(0);
}

char *
mkmsg2( const char *fmt, ...) {
  int size;
  va_list ap;
  static char buff[MAXLINE+1];
  
  size = MAXLINE;
  va_start(ap, fmt);
  vsnprintf(buff, size, fmt, ap);
  va_end(ap);

  return(buff);
}


/* usaged : print handling instructions of wthd */
int
usaged (int exitcode, char *error, char *addl) {
      fprintf(stderr,"Usage: wthd [options]\n"
                     "where options include:\n"
                     "\t-d\t\tdo not daemonize, stay in foreground\n"
                     "\t-f <filename>\tuse filename as configuration file\n"
                     "\t-p <portnumber>\tuse portnumber to listen\n"
                     "\t-h\t\tprint this help\n");

      if (strlen(error) > 0) fprintf(stderr, "%s: %s\n", error, addl);
      exit(exitcode);
}


/* readconfig : read configuration file */
int
readconfig( ) {
  int cfgok;
  FILE *cfg;
  char line[BUFFSIZE];
  char *name;
  char *value;
  char *cp, *cp2;

  cfgok = FALSE;
  if ( wsconf.configfile != NULL ) {
    printf("wsconf.configfile: %s\n", wsconf.configfile);
    if ( ( cfg = fopen(wsconf.configfile,"r")) != NULL ) {
      printf("Reading config file %s\n", wsconf.configfile);
      cfgok = TRUE;
    }
  } else { 
    if ( ( cfg = fopen("/etc/wth/wth.conf","r")) != NULL ) {
      printf("Reading config file /etc/wth/wth.conf\n");
      cfgok = TRUE;
    }
    else if ( ( cfg = fopen("/usr/local/etc/wth/wth.conf","r")) != NULL ) {
      printf("Reading config file /usr/local/etc/wth/wth.conf\n");
      cfgok = TRUE;
    }
  }

  if ( cfgok == FALSE ) {
    printf("No config file found, using default parameters\n");
    return(-1);
  }

  /* stolen from thttpd code */
  while ( fgets(line, sizeof(line), cfg) != NULL) {
	/* Trim comments. */
    if ( ( cp = strchr( line, '#' ) ) != NULL )
	  *cp = '\0';
	/* or empty line */
    if ( ( cp = strchr( line, '\n' ) ) != NULL )
        *cp = '\0';
		
    /* Split line into words. */
    for ( cp = line; *cp != '\0'; cp = cp2 )
	  {
        /* Skip leading whitespace. */
        cp += strspn( cp, " \t\n\r" );
        /* Find next whitespace. */
        cp2 = cp + strcspn( cp, " \t\n\r" );
        name = cp;
        value = cp2;
        if ( value != (char*) 0 )
		  *value++ = '\0';
		
        /* Interpret. */		
        if ( strcasecmp( name, "timeout" ) == 0 ) {
		  wsconf.timeout = atoi(value);
        } else if ( strcasecmp( name, "port" ) == 0 ) {
	  wsconf.port = strdup(value);
        } else if ( strcasecmp( name, "ws2000.device" ) == 0 ) {
	  printf("ws2000.device: \"%s\"\n", value);
	  strncpy(ws2000station.config.device, value, MAXMSGLEN);
        } else if ( strcasecmp( name, "ws2000.dbfile" ) == 0 ) {
	  strncpy( ws2000station.config.dbfile, strdup(value), TBUFF);
	  printf("ws2000.dbfile: \"%s\"\n", value);
        } else if ( strcasecmp( name, "ws2000.monitor" ) == 0 ) {
	  strncpy( ws2000station.config.monitor, strdup(value), TBUFF);
	  printf("ws2000.monitor: \"%s\"\n", value);
        } else if ( strcasecmp( name, "ws2000.rrdpath" ) == 0 ) {
	  strncpy( ws2000station.config.rrdpath, strdup(value), SBUFF);
	  printf("ws2000.rrdpath: \"%s\"\n", value);

        } else if ( strcasecmp( name, "pcwsr.device" ) == 0 ) {
	  printf("pcwsr.device: \"%s\"\n", value);
	  strncpy(pcwsrstation.config.device, value, MAXMSGLEN);
        } else if ( strcasecmp( name, "pcwsr.dbfile" ) == 0 ) {
	  strncpy(pcwsrstation.config.dbfile, strdup(value), TBUFF);
	  printf("pcwsr.dbfile: \"%s\"\n", value);
        } else if ( strcasecmp( name, "pcwsr.monitor" ) == 0 ) {
	  strncpy(pcwsrstation.config.monitor, strdup(value), TBUFF);
	  printf("pcwsr.monitor: \"%s\"\n", value);
        } else if ( strcasecmp( name, "pcwsr.rrdpath" ) == 0 ) {
	  strncpy( pcwsrstation.config.rrdpath, strdup(value), SBUFF);
	  printf("pcwsr.rrdpath: \"%s\"\n", value);

        } else if ( strcasecmp( name, "onewire.device" ) == 0 ) {
	  printf("onewire.device: \"%s\"\n", value);
	  strncpy(onewirestation.config.device, value, MAXMSGLEN);
        } else if ( strcasecmp( name, "onewire.dbfile" ) == 0 ) {
	  printf("onewire.dbfile: \"%s\"\n", value);
	  strncpy(onewirestation.config.dbfile, strdup(value), TBUFF);
        } else if ( strcasecmp( name, "onewire.rrdpath" ) == 0 ) {
	  printf("onewire.rrdpath: \"%s\"\n", value);
	  strncpy(onewirestation.config.rrdpath, strdup(value), TBUFF);
        } else if ( strcasecmp( name, "onewire.mcycle" ) == 0 ) {
	  printf("onewire.mcycle: \"%s\"\n", value);
	  onewirestation.config.mcycle = atoi(value);

        } else if ( strcasecmp( name, "wmr9x8.device" ) == 0 ) {
	  printf("wmr9x8.device: \"%s\"\n", value);
	  strncpy(wmr9x8station.config.device, value, MAXMSGLEN);
        } else if ( strcasecmp( name, "wmr9x8.dbfile" ) == 0 ) {
	  printf("wmr9x8.dbfile: \"%s\"\n", value);
	  strncpy(wmr9x8station.config.dbfile, strdup(value), TBUFF);
        } else if ( strcasecmp( name, "wmr9x8.rrdpath" ) == 0 ) {
	  printf("wmr9x8.rrdpath: \"%s\"\n", value);
	  strncpy(wmr9x8station.config.rrdpath, strdup(value), TBUFF);
        } else if ( strcasecmp( name, "wmr9x8.mcycle" ) == 0 ) {
	  printf("wmr9x8.mcycle: \"%s\"\n", value);
	  wmr9x8station.config.mcycle = atoi(value);

        } else if ( strcasecmp( name, "umeter.device" ) == 0 ) {
	  printf("umeter.device: \"%s\"\n", value);
	  strncpy(umeterstation.config.device, value, MAXMSGLEN);
        } else if ( strcasecmp( name, "umeter.dbfile" ) == 0 ) {
	  printf("umeter.dbfile: \"%s\"\n", value);
	  strncpy(umeterstation.config.dbfile, strdup(value), TBUFF);
        } else if ( strcasecmp( name, "umeter.rrdpath" ) == 0 ) {
	  printf("umeter.rrdpath: \"%s\"\n", value);
	  strncpy(umeterstation.config.rrdpath, strdup(value), TBUFF);
        } else if ( strcasecmp( name, "umeter.mcycle" ) == 0 ) {
	  printf("umeter.mcycle: \"%s\"\n", value);
	  umeterstation.config.mcycle = atoi(value);

        } else {
	  printf("unknown option '%s' inf configuration file\n", name );
          return(-1);
        }
    }		
  }
  return(0);  
}


/* echoconfig

   echo datastructure cmd
*/
char *
echoconfig ( char *station) {
  int err;
  char s[TBUFF+1];
  static char t[SBUFF+1];

  snprintf(t, SBUFF, "");
  printf("echoconfig: station: \"%s\"\n", station);

  snprintf(s, TBUFF,"global configuration\n");
  strncat(t,s,strlen(s));
  snprintf(s, TBUFF, "\ttimeout serialline\t%d\n",wsconf.timeout);
  strncat(t,s, strlen(s));
  snprintf(s, TBUFF, "\ttelnet port\t\t%s\n",wsconf.port);
  strncat(t,s, strlen(s));
 
  if ( ( err = strncmp( station, "ws2000", 5)) == 0 ) {
    snprintf(s, TBUFF, 
      "ws2000 configuration\n");
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tdevice\t\t\t%s\n",ws2000station.config.device);
    strncat(t,s, strlen(s));
    if ( strncmp( ws2000station.config.device, "n.a.", 4) == 0) 
      return(t);
    snprintf(s, TBUFF, 
      "\tdatabase\t\t%s\n",ws2000station.config.dbfile);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\trrdpath\t\t\t%s\n",ws2000station.config.rrdpath);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tmonitor\t\t\t%s\n",ws2000station.config.monitor);
    strncat(t,s, strlen(s));
  } else if ( ( err = strncmp( station, "pcwsr", 5)) == 0 ) {
    snprintf(s, TBUFF, 
      "pcwsr configuration\n");
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tdevice\t\t\t%s\n",pcwsrstation.config.device);
    strncat(t,s, strlen(s));
    if ( strncmp( pcwsrstation.config.device, "n.a.", 4) == 0) 
      return(t);
    snprintf(s, TBUFF,
      "\tdatabase\t\t%s\n",pcwsrstation.config.dbfile);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\trrdpath\t\t\t%s\n",pcwsrstation.config.rrdpath);
    strncat(t,s, strlen(s));
  } else if ( ( err = strncmp( station, "onewire", 5)) == 0 ) {
    snprintf(s, TBUFF, 
      "1wire configuration\n");
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tdevice\t\t\t%s\n",onewirestation.config.device);
    strncat(t,s, strlen(s));
    if ( strncmp( onewirestation.config.device, "n.a.", 4) == 0) 
      return(t);
    snprintf(s, TBUFF,
      "\tdatabase\t\t%s\n",onewirestation.config.dbfile);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\trrdpath\t\t\t%s\n",onewirestation.config.rrdpath);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tmeasurement cycles\t%d\n",onewirestation.config.mcycle);
    strncat(t,s, strlen(s));
  } 
  return(t);
}

Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0) {
	  return(SIG_ERR);
	}
	return(oact.sa_handler);
}
/* end signal */


/*
  catch signal routine
*/
void 
*signal_hd( void *arg) 
{
  sigset_t signals_to_catch;
  int caught;


  sigemptyset( &signals_to_catch);
  sigaddset( &signals_to_catch, SIGUSR1);
  sigwait( &signals_to_catch, &caught);

  /* got SIGUSR1 - start to stop wthd */
  pthread_mutex_lock( &pthread_info.mutex);

  pthread_info.received_shutdown_req = TRUE;

  /* wait for in-progress requests threads to finish */
  while ( pthread_info.num_active > 0 ) {
    pthread_cond_wait( &pthread_info.thread_exit_cv, 
		       &pthread_info.mutex);
  }
  pthread_mutex_unlock( &pthread_info.mutex);
  syslog(LOG_ALERT, "signal_hd: received signal: shutting down");
  exit(0);
  return(NULL);
}

