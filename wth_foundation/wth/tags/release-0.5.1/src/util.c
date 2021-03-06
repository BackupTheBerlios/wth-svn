/* util.c

   $Id: util.c 213 2008-10-25 20:33:26Z vjahns $
   $Revision: 213 $

   Copyright 2007,2008 Volker Jahns <volker@thalreit.de>

   collection of utility functions

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
  wsconf.port        = "2000";
  wsconf.wwwport     = "8880";
  wsconf.units       = "SI";
  wsconf.outfmt      = "old";
  
  strncpy(ws2000station.config.dbfile, "ws2000.db", TBUFF);
  strncpy(ws2000station.config.device, "n.a.", TBUFF);
  strncpy(ws2000station.config.rrdpath, ".", TBUFF);
  strncpy(ws2000station.config.monitor,"Sensormonitor.rrd", TBUFF);;
  ws2000station.status.interval      = 300;  

  strncpy(pcwsrstation.config.dbfile, "pcwsr.db", TBUFF);
  strncpy( pcwsrstation.config.device, "n.a.", TBUFF);
  err = readconfig();
  printf("wthd_init: readconfig done\n");

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
  return(0);
}


/* echodata
  
   print out raw data frame reveived from weather station 

*/
int
echodata(unsigned char *data, int mdat) {
    int i;
    char frame[NBUFF] = "";
    char sf[3] = "";

    syslog(LOG_DEBUG, "echodata : length dataframe : %d\n",mdat);

    /* for better readability label each byte in frame */    
    for ( i = 0; i < mdat; i++ ) {
      snprintf(sf, 4, "%2d:",i);
      strncat(frame, sf, 4);
    }
    syslog(LOG_DEBUG, "echodata : %s\n", frame);    
    strcpy(frame, "");

    for ( i = 0; i < mdat; i++ ) {
      snprintf(sf, 4, "%2x:",data[i]);
      strncat(frame, sf, 4);
    }

    syslog(LOG_DEBUG, "echodata : %s\n", frame);   

    return(0);
} 

/* 
   getbits

   transfrom bytes of weather station data to BCD, otherwise picking single
   or group of bits.
   Kerninghan, Ritchie, The C Programming Language, p49.
*/
unsigned char
getbits(unsigned char x, int p, int n) {
  return ( x>>(p+1-n)) & ~(~0 <<n);
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
      fprintf(stderr,"Usage: wthd [Options]\n"
                     "where options include:\n"
                     "\t-d \t\tdo not daemonize, stay in foreground\n"
                     "\t-p <portnumber>\tuse portnumber to listen\n");

      if (error) fprintf(stderr, "%s: %s\n", error, addl);
      exit(exitcode);
}


/* readconfig : read configuration file */
int
readconfig( ) {
  FILE *cfg;
  char line[BUFFSIZE];
  char *name;
  char *value;
  char *cp, *cp2;
  char *cfgfile;

  if ( ( cfg = fopen("/etc/wth/wth.conf","r")) != NULL ) {
    cfgfile = "/etc/wth/wth.conf";
    printf("Reading config file /etc/wth/wth.conf\n");
  }
  else if ( ( cfg = fopen("/usr/local/etc/wth/wth.conf","r")) != NULL ) {
    cfgfile = "/usr/local/etc/wth.conf";
    printf("Reading config file /usr/local/etc/wth/wth.conf\n");
  }
  else {
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
        } else if ( strcasecmp( name, "logfacility" ) == 0 ) {
	  if ( strcasecmp( value, "local0") == 0 ) 
	    wsconf.logfacility = LOG_LOCAL0;
	  else if ( strcasecmp( value, "local1") == 0 ) 
	    wsconf.logfacility = LOG_LOCAL1;
	  else if ( strcasecmp( value, "local2") == 0 ) 
	    wsconf.logfacility = LOG_LOCAL2;
	  else if ( strcasecmp( value, "local3") == 0 ) 
	    wsconf.logfacility = LOG_LOCAL3;
	  else if ( strcasecmp( value, "local4") == 0 ) 
	    wsconf.logfacility = LOG_LOCAL4;
	  else if ( strcasecmp( value, "local5") == 0 ) 
	    wsconf.logfacility = LOG_LOCAL5;
	  else if ( strcasecmp( value, "local6") == 0 ) 
	    wsconf.logfacility = LOG_LOCAL6;
	  else if ( strcasecmp( value, "local7") == 0 ) 
	    wsconf.logfacility = LOG_LOCAL7;
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
	  /* this is not complete and 
	     has to be changed
	  */
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
  snprintf(s, TBUFF, "\tsyslog logfacility\t%d\n",wsconf.logfacility);
  strncat(t,s, strlen(s));
  snprintf(s, TBUFF, "\ttelnet port\t\t%s\n",wsconf.port);
  strncat(t,s, strlen(s));
 
  if ( ( err = strncmp( station, "ws2000", 5)) == 0 ) {
    snprintf(s, TBUFF, 
      "ws2000 configuration\n");
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tws2000 device\t\t%s\n",ws2000station.config.device);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tws2000 database\t\t%s\n",ws2000station.config.dbfile);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tws2000 rrdpath\t\t%s\n",ws2000station.config.rrdpath);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tws2000 monitor\t\t%s\n",ws2000station.config.monitor);
    strncat(t,s, strlen(s));
  } else if ( ( err = strncmp( station, "pcwsr", 5)) == 0 ) {
    snprintf(s, TBUFF, 
      "pcwsr configuration\n");
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tpcwsr device\t\t%s\n",pcwsrstation.config.device);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF,
      "\tpcwsr database\t\t%s\n",pcwsrstation.config.dbfile);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\tpcwsr rrdpath\t\t%s\n",pcwsrstation.config.rrdpath);
    strncat(t,s, strlen(s));
  } else if ( ( err = strncmp( station, "onewire", 5)) == 0 ) {
    snprintf(s, TBUFF, 
      "1wire configuration\n");
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\t1wire device\t%s\n",onewirestation.config.device);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF,
      "\t1wire database\t%s\n",onewirestation.config.dbfile);
    strncat(t,s, strlen(s));
    snprintf(s, TBUFF, 
      "\t1wire rrdpath\t%s\n",onewirestation.config.rrdpath);
    strncat(t,s, strlen(s));
  } 
  return(t);
}

Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

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

