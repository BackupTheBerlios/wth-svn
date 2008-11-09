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
  wsconf.netflg      = 0;
  wsconf.hostname    = "localhost";
  wsconf.port        = "2000";
  wsconf.wwwport     = "8880";
  wsconf.units       = "SI";
  wsconf.outfmt      = "old";
  
  ws2000station.config.dbfile        = "ws2000.db";
  strncpy(ws2000station.config.device, "n.a.", MAXMSGLEN);
  ws2000station.config.rrdpath       = ".";
  ws2000station.config.monitor       = "Sensormonitor.rrd";
  ws2000station.status.interval      = 300;  

  pcwsrstation.config.dbfile         = "pcwsr.db";
  strncpy( pcwsrstation.config.device, "n.a.", MAXMSGLEN);
  err = readconfig();
  printf("wthd_init: readconfig done\n");

  /* allocate sensornames WS2000 */
  if (( ws2000station.sensor[1].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[1].sensorname, "Sensor1", MAXMSGLEN);

  if (( ws2000station.sensor[2].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[2].sensorname, "Sensor2", MAXMSGLEN);

  if (( ws2000station.sensor[3].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[3].sensorname, "Sensor3", MAXMSGLEN);

  if (( ws2000station.sensor[4].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[4].sensorname, "Sensor4", MAXMSGLEN);

  if (( ws2000station.sensor[5].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[5].sensorname, "Sensor5", MAXMSGLEN);

  if (( ws2000station.sensor[6].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[6].sensorname, "Sensor6", MAXMSGLEN);

  if (( ws2000station.sensor[7].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[7].sensorname, "Sensor7", MAXMSGLEN);

  if (( ws2000station.sensor[8].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[8].sensorname, "Sensor8", MAXMSGLEN);

  if (( ws2000station.sensor[9].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[9].sensorname, "Rainsensor", MAXMSGLEN);

  if (( ws2000station.sensor[10].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[10].sensorname, "Windsensor", MAXMSGLEN);

  if (( ws2000station.sensor[11].sensorname = malloc(MAXMSGLEN)) == NULL)
    return(-1);
  strncpy(ws2000station.sensor[11].sensorname, "Indoorsensor", MAXMSGLEN);

  /*
  rbuf = malloc(2*MAXMSGLEN*sizeof(char *));
  strncpy( rbuf, echoconfig(), MAXMSGLEN);
  printf("Configuration:\n%s", rbuf);
  */
  return(0);
}


/* echodata
  
   print out raw data frame reveived from weather station 

*/
int
echodata(unsigned char *data, int mdat) {
    int i;
    char frame[MAXBUFF] = "";
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

/* usage : print handling instructions of wthc */
int
usage (int exitcode, char *error, char *addl) {
  char *bufstr;

  if (( bufstr = malloc(MAXMSGLEN)) == NULL ) 
  {
    return(1);
  }

  snprintf( bufstr, MAXMSGLEN, "Usage: wthc [Options]\n"
	  "where options include:\n"
	  "\t-c <command>\texecute command\n"
	  "\t\t0\tPoll DCF Time\n"
	  "\t\t1\tRequest dataset\n"
	  "\t\t2\tSelect next dataset\n"
	  "\t\t3\tActivate 9 temperature sensors\n"
	  "\t\t4\tActivate 16 temperature sensors\n"
	  "\t\t5\tRequest status\n"
	  "\t\t6\tSet interval time,\n" 
	   "\t\t\t\trequires extra parameter specified by option -i\n");
  snprintf( bufstr, MAXMSGLEN, "%s"
	  "\t\t12\tRequest all available data recursively\n"
	  "\t-i <interval>\tspecifies interval time\n"
	  "\t\tpermissible values for the interval lie\n"
	  "\t\twithin the range from 1 to 60 minutes\n"
	  "\t-h <hostname>\tconnect to <hostname/ipaddress>\n"
	  "\t-p <portnumber>\tuse portnumber at remote host\n"
	  "\t\tfor connection\n"
	  "\t-s\t\tuse local serial connection\n"
	  "\t-t\t\tset time using internal DCF77 receiver\n"
	  "\t\tneeds superuser privileges\n" 
	  "\t-u <units>\tset the units of measured values\n"
          "\t\tSI\tuse SI units for output: °C, m/s, hPa\n"
          "\t\tUS\tuse US units for output: °F, mph, inHg\n"
  ,bufstr);

  fprintf(stderr, "%s", bufstr);

  if (error) fprintf(stderr, "%s: %s\n", error, addl);
  exit(exitcode);
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
	  ws2000station.config.dbfile = strdup(value);
	  printf("ws2000.dbfile: \"%s\"\n", value);
        } else if ( strcasecmp( name, "ws2000.monitor" ) == 0 ) {
	  ws2000station.config.monitor = strdup(value);
	  printf("ws2000.monitor: \"%s\"\n", value);
        } else if ( strcasecmp( name, "ws2000.rrdpath" ) == 0 ) {
	  ws2000station.config.rrdpath = strdup(value);
	  printf("ws2000.rrdpath: \"%s\"\n", value);
        } else if ( strcasecmp( name, "pcwsr.device" ) == 0 ) {
	  printf("pcwsr.device: \"%s\"\n", value);
	  strncpy(pcwsrstation.config.device, value, MAXMSGLEN);
        } else if ( strcasecmp( name, "pcwsr.dbfile" ) == 0 ) {
	  pcwsrstation.config.dbfile = strdup(value);
	  printf("pcwsr.dbfile: \"%s\"\n", value);
        } else if ( strcasecmp( name, "pcwsr.monitor" ) == 0 ) {
	  pcwsrstation.config.monitor = strdup(value);
	  printf("pcwsr.monitor: \"%s\"\n", value);
        } else if ( strcasecmp( name, "pcwsr.rrdpath" ) == 0 ) {
	  pcwsrstation.config.rrdpath = strdup(value);
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
echoconfig ( ) {
  int size;
  char *s;
  static char t[MAXLINE];

  s = mkmsg2("Configuration parameters\n");
  size = strlen(s) + 1;
  
  strncat(t,s,strlen(s));
  s = mkmsg2("\twsconf.command: %d\n", wsconf.command);
  strncat(t,s, strlen(s));

  s = mkmsg2("\twsconf.argcmd: %d\n",wsconf.argcmd);
  strncat(t,s, strlen(s));

  s = mkmsg2("\twsconf.netflg: %d\n",wsconf.netflg);
  strncat(t,s, strlen(s));
  s = mkmsg2("\twsconf.verbose: %d\n",wsconf.verbose);
  strncat(t,s, strlen(s));
  s = mkmsg2("\twsconf.timeout: %d\n",wsconf.timeout);
  strncat(t,s, strlen(s));
  s = mkmsg2("\twsconf.logfacility: %d\n",wsconf.logfacility);
  strncat(t,s, strlen(s));
  s = mkmsg2("\twsconf.hostname: %s\n",wsconf.hostname);
  strncat(t,s, strlen(s));
  s = mkmsg2("\twsconf.port: %s\n",wsconf.port);
  strncat(t,s, strlen(s));
  
  s = mkmsg2("\tws2000station.config.device:\t%s\n",ws2000station.config.device);
  strncat(t,s, strlen(s));
 
  s = mkmsg2("\tws2000station.config.dbfile:\t%s\n",ws2000station.config.dbfile);
  strncat(t,s, strlen(s));
  s = mkmsg2("\tpcwsrstation.config.device:\t%s\n",pcwsrstation.config.device);
  strncat(t,s, strlen(s));
  s = mkmsg2("\tpcwsrstation.config.dbfile:\t%s\n",pcwsrstation.config.dbfile);
  strncat(t,s, strlen(s));
  
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

