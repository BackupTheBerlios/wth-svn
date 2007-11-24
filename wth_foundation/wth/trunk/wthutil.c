/* wthutil.c

   $Id$
   $Revision$

   Copyright 2007 Volker Jahns <volker@thalreit.de>

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
#include <wth.h>


/* pdata 

print the sensor data as hold in structure sens

*/
char *
pdata(struct wthio *wth, struct cmd *pcmd) {
  int i,j; 
  int size = 100;
  static char t[MAXBUFF];
  char *s;
  struct tm tm_buff;

  if ( strncmp(pcmd->outfmt,"old",3) == 0) {
    t[0] = '\0';
    s = mkmsg("Sensor\tType\tStatus\tdataset\ttime\t\tsign\tabs.value\n");
    strncat(t, s, strlen(s));
  
    for ( i = 0; i < MAXSENSORS; i++ ) {
      if ( wth->sens[i].status != 0 ) {   
	for ( j = 0; j < wth->wstat.ndats; j++) {

	  s = mkmsg("%d\t%s\t%d\t%d\t%lu\t%d\t%8.1f\n", i, 
		    wth->sens[i].type, wth->sens[i].status, j, 
		    wth->sens[i].mess[j].time,
		    wth->sens[i].mess[j].sign,
		    wth->sens[i].mess[j].value);
	
	  size = size + strlen(s) + 1;
	  strncat(t, s, strlen(s));
	}
      }
    }
  } else if ( strncmp(pcmd->outfmt,"std",3) == 0) {
      t[0] = '\0';
      s = mkmsg("Not implemented\n");
      strncat(t, s, strlen(s));
  } else if ( strncmp(pcmd->outfmt,"lline",5) == 0) {
#ifdef YYY
    strcpy(t, mkmsg("#time"));
    size = strlen(t);

    for ( i = 0; i < MAXSENSORS; i++ ) {
#ifdef XXX
      if ( wth->sens[i].status != 0 )
	{        s = mkmsg("\t%s_%d", wth->sens[i].type, i );
#else
	{       s = mkmsg("\t%d", i);
#endif
	size = size + strlen(s) + 1;
	strncat(t, s, strlen(s));
	}
	}
      size++;
      strncat(t,"\n",1);
#else
      t[0] = '\0';
#endif
      localtime_r(&wth->sens[0].mess[0].time, &tm_buff);
      s = mkmsg("%04d-%02d-%02d %02d:%02d:%02d", tm_buff.tm_year+1900,tm_buff.tm_mon+1,tm_buff.tm_mday,tm_buff.tm_hour,tm_buff.tm_min,tm_buff.tm_sec);
      size = size + strlen(s) + 1;
      strncat(t, s, strlen(s));

      for ( i = 0; i < MAXSENSORS; i++ ) {
	for ( j = 0; j < wth->wstat.ndats; j++) {
	  if ( wth->sens[i].status != 0 )
	    {
              s = mkmsg("\t%.1f", wth->sens[i].mess[j].value);
	    }
	  else
	    s = "\t-";
	  size = size + strlen(s) + 1;
	  strncat(t, s, strlen(s));
	}

      }

      size++;
      strncat(t,"\n",1);
      return (t);
    }

    return (t);
  }

#if defined POSTGRES
/* pg_data

insert sensor data into postgresql database

*/
int 
pg_data ( struct cmd *pcmd, struct wthio *wth) 
{
  int j;

  /* database variables */
  char *conninfo;
  PGconn *conn;
  PGresult *res;
  int humi, humo, bar, windd, rain;
  float tempi, tempo, winds;
  char sqlstr[300];

  time_t tpc; 
  struct tm *tmpc; 
  char timebuff[40] = "";

  int conncount;

  /* make a connection to the database "weather" */
  if (( conninfo = malloc(MAXBUFF)) == NULL )
    return 1;

  snprintf( conninfo, MAXBUFF, "dbname=%s user=%s", 
	    pcmd->database, pcmd->dbuser);
  conn = PQconnectdb(conninfo);

  /* Check to see that the backend connection was successfully made */
  if (PQstatus(conn) != CONNECTION_OK){
    syslog(LOG_INFO, "pg_data: Connection to database failed: %s",
	    PQerrorMessage(conn));
    PQfinish(conn);
    return(1);
  }

  /* reset data logger access command counter */
  conncount=0;

  /* Use simple variable names for the data just retreived */

  for ( j = 0; j < wth->wstat.ndats; j++) {
    tempo = -wth->sens[0].mess[j].value;
    if(!wth->sens[0].mess[j].sign){
      tempo = -tempo;
    }
    humo = (int)wth->sens[1].mess[j].value;
    rain = (int)wth->sens[16].mess[j].value;
    winds = wth->sens[17].mess[j].value;
    windd = (int)wth->sens[18].mess[j].value;
    bar = (int)wth->sens[20].mess[j].value;
    tempi = -wth->sens[21].mess[j].value;
    if(!wth->sens[21].mess[j].sign){
      tempi = -tempi;
    }
    humi = (int)wth->sens[22].mess[j].value;
      
    tpc = wth->sens[20].mess[j].time;
    tmpc = localtime(&tpc);
    strftime(timebuff, sizeof(timebuff), "%Y-%m-%d %X", tmpc);

    /* Write data out to the database */
    snprintf(sqlstr, MAXBUFF,
	    "INSERT into weather (" 
	    "tempo,"
	    "humo,"
	    "rain,"
	    "winds,"
	    "windd,"
	    "bar,"
	    "tempi,"
	    "humi,"
	    "t )"
	    " values ( '%3.1f', "
	    "'%3d','%4d','%3.1f','%3d','%3d','%3.1f','%3d','%s')",
	    tempo,humo,rain,winds,windd,bar,tempi,humi,timebuff);
    syslog(LOG_DEBUG, "pg_data: sqlstr: %s\n",sqlstr);
      
    res = PQexec(conn, sqlstr);
    if( PQresultStatus( res ) != PGRES_COMMAND_OK ) { 
      syslog(LOG_INFO, "pg_data: PQexec failed: %s", PQerrorMessage( conn ) );
      PQfinish(conn);
      return(1);
    }
    PQclear(res);
  }

  /* Clean up database connection when exiting */
  PQfinish(conn);
  return(0);

}
#endif


/* initdata 

presets the datastructures
rw->sens
pcmd

*/
int initdata(struct wthio *rw) {
  int i;  /* array subscription */  

  /* set type of the sensors */
  /* first 8 temperature/humidity sensors */
  for ( i = 0; i < 8; i++) {
    rw->sens[2*i].type   = "temp";
    rw->sens[2*i+1].type = "hum";
  }
  
  /* rain sensor */
  rw->sens[16].type  = "rain";
  /* wind sensor */
  rw->sens[17].type  = "wspd";
  rw->sens[18].type  = "wdir";
  rw->sens[19].type  = "wdev";
  /* indoor sensor : temp, hum, press */
  rw->sens[20].type  = "pres";
  rw->sens[21].type  = "temp";
  rw->sens[22].type  = "hum";
  /* sensor 9 is combined temperature/humidity */
  rw->sens[23].type  = "temp";
  rw->sens[24].type  = "hum";
      
  /* status of pressure/temperature/humidity sensor 10 to 15 */
  for ( i = 0; i < 18; i = i + 3) {
    rw->sens[25 + i    ].type = "pres";
    rw->sens[25 + i + 1].type = "temp";
    rw->sens[25 + i + 2].type = "hum";
  }
  return(0);
}


/* initcmd

presets the datastructures
pcmd

*/
struct cmd
*initcmd( void) {  

  struct cmd *inipcmd;   
  /* initialize struct pcmd */
  inipcmd = ( struct cmd *) malloc(sizeof(struct cmd *));
  inipcmd->argcmd      = 0;
  inipcmd->timeout     = TIMEOUT;
  inipcmd->baudrate    = BAUDRATE;
  inipcmd->logfacility = LOGFACILITY;
  inipcmd->device      = SER_DEVICE;
  inipcmd->verbose     = 0;
  inipcmd->netflg      = -1;
  inipcmd->hostname    = "localhost"; 
  inipcmd->port        = WPORT; 
  inipcmd->tnport      = TNPORT;   
  inipcmd->wstype      = WSTYPE;
  inipcmd->xmlport     = XMLPORT;
  inipcmd->database    = DATABASE;
  inipcmd->dbuser      = DBUSER;
  inipcmd->units       = UNITS;
  inipcmd->outfmt      = OUTFMT;

  return(inipcmd);
  //return(0);
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
	  sprintf(sf, "%2d:",i);
      strcat(frame, sf);
    }
    syslog(LOG_DEBUG, "echodata : %s\n", frame);    
    strcpy(frame, "");

    for ( i = 0; i < mdat; i++ ) {
	  sprintf(sf, "%2x:",data[i]);
      strcat(frame, sf);
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


/* mkmsg

   man snprintf (LINUX) sample code modified
   
   old version - does malloc lead to seg fault?
*/
char *
mkmsg(const char *fmt, ...) {
   int n, size = 1024;
   char *p;
   va_list ap;
   if ((p = malloc(MAXBUFF)) == NULL)
      return NULL;
   //printf("mkmsg: alloc p OK\n");
   while (1) {
      va_start(ap, fmt);
      n = vsnprintf (p, size, fmt, ap);
      va_end(ap);
      if (n > -1 && n < size)
         return (p);
      if (n > -1)    /* LINUX glibc 2.1, FreeBSD */ 
		size = n+1;  
	  else           /* LINUX glibc 2.0 */
        size *= 2;  /* twice the old size */
      if ((p = realloc (p, size)) == NULL)
         return NULL;
   }
}



/* usage : print handling instructions of wthc */
int
usage (int exitcode, char *error, char *addl) {
  char *bufstr;

  if (( bufstr = malloc(MAXBUFF)) == NULL ) 
  {
    return(1);
  }

  snprintf( bufstr, MAXBUFF, "Usage: wthc [Options]\n"
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
#ifdef POSTGRES
  snprintf( bufstr, MAXBUFF, "%s" 
	  "\t\t7\tEvery 30 seconds, request data from the data\n"
	  "\t\t\tlogger,and store it into the weather table of a\n"
	   "\t\t\tpostgres database named weather\n", bufstr);
#endif
  snprintf( bufstr, MAXBUFF, "%s"
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

/* tnusage : print handling instructions for telnet access wthd */
char *
tnusage (int exitcode, char *error, char *addl) {
  char *s;

  s = mkmsg("Available commands include:\n"
                     "\t0\t\tPoll DCF Time\n"
                     "\t1\t\tRequest dataset\n"
                     "\t2\t\tSelect next dataset\n"
                     "\t3\t\tActivate 9 temperature sensors\n"
                     "\t4\t\tActivate 16 temperature sensors\n"
                     "\t5\t\tRequest status\n"
                     "\t6 <interval>\tSet <interval> time, " 
                     "requires extra parameter\n"
		     "\t\t\tpermissible values for <interval> lie\n"
		     "\t\t\twithin the range from <1> to <60> minutes\n");

      return(s);
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
char *
readconfig(struct cmd *pcmd) {
  int ival;
  FILE *cfg;
  char line[BUFFSIZE];
  char *name;
  char *value;
  char *cp, *cp2;
  char *rbuf;
  char *cfgfile;

   if ( ( cfg = fopen("/etc/wth/wth.conf","r")) != NULL ) {
    cfgfile = "/etc/wth/wth.conf";
  }
  else if ( ( cfg = fopen("/usr/local/etc/wth/wth.conf","r")) != NULL ) {
    cfgfile = "/usr/local/etc/wth.conf";
  }
  else {
    rbuf = mkmsg("No configuration file found, using default parameters\n");
    return(rbuf);
  }

  rbuf = mkmsg("Using config file: %s\n", cfgfile);

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
		  pcmd->timeout = atoi(value);
        } else if ( strcasecmp( name, "baudrate" ) == 0 ) {
          ival = atoi(value);     
		  switch (ival) {
		  case     50: pcmd->baudrate =     B50; break;
		  case     75: pcmd->baudrate =     B75; break;
		  case    110: pcmd->baudrate =    B110; break;
		  case    134: pcmd->baudrate =    B134; break;
		  case    150: pcmd->baudrate =    B150; break;
		  case    200: pcmd->baudrate =    B200; break;
		  case    300: pcmd->baudrate =    B300; break;
		  case    600: pcmd->baudrate =    B600; break;
		  case   1200: pcmd->baudrate =   B1200; break;
		  case   1800: pcmd->baudrate =   B1800; break;
		  case   2400: pcmd->baudrate =   B2400; break;
		  case   4800: pcmd->baudrate =   B4800; break;
		  case   9600: pcmd->baudrate =   B9600; break;
		  case  19200: pcmd->baudrate =  B19200; break;
		  case  38400: pcmd->baudrate =  B38400; break;
		  case  57600: pcmd->baudrate =  B57600; break;
		  case 115200: pcmd->baudrate = B115200; break;
		  default:      pcmd->baudrate =  B9600; break; /* keep gcc happy */
		  }		  
        } else if ( strcasecmp( name, "logfacility" ) == 0 ) {
		    if ( strcasecmp( value, "kern") == 0 ) 
			  pcmd->logfacility = LOG_KERN;
			else if ( strcasecmp( value, "user") == 0 ) 
			  pcmd->logfacility = LOG_USER;
			else if ( strcasecmp( value, "mail") == 0 ) 
			  pcmd->logfacility = LOG_MAIL;     
			else if ( strcasecmp( value, "daemon") == 0 ) 
			  pcmd->logfacility = LOG_DAEMON;
			else if ( strcasecmp( value, "syslog") == 0 ) 
			  pcmd->logfacility = LOG_SYSLOG;
			else if ( strcasecmp( value, "lpr") == 0 ) 
			  pcmd->logfacility = LOG_LPR;
			else if ( strcasecmp( value, "news") == 0 ) 
			  pcmd->logfacility = LOG_NEWS;
			else if ( strcasecmp( value, "uucp") == 0 ) 
			  pcmd->logfacility = LOG_UUCP;
			else if ( strcasecmp( value, "cron") == 0 ) 
			  pcmd->logfacility = LOG_CRON;
			else if ( strcasecmp( value, "ftp") == 0 ) 
			  pcmd->logfacility = LOG_FTP;
			else if ( strcasecmp( value, "local0") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL0;
			else if ( strcasecmp( value, "local1") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL1;
			else if ( strcasecmp( value, "local2") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL2;
			else if ( strcasecmp( value, "local3") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL3;
			else if ( strcasecmp( value, "local4") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL4;
			else if ( strcasecmp( value, "local5") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL5;
			else if ( strcasecmp( value, "local6") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL6;
			else if ( strcasecmp( value, "local7") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL7;
        } else if ( strcasecmp( name, "device" ) == 0 ) {
		  pcmd->device = strdup(value);
        } else if ( strcasecmp( name, "port" ) == 0 ) {
		  pcmd->port = strdup(value);
        } else if ( strcasecmp( name, "tnport" ) == 0 ) {
		  pcmd->tnport = strdup(value);
        } else if ( strcasecmp( name, "xmlport" ) == 0 ) {
		  pcmd->xmlport = strdup(value);
        } else if ( strcasecmp( name, "database" ) == 0 ) {
		  pcmd->database = strdup(value);
        } else if ( strcasecmp( name, "dbuser" ) == 0 ) {
		  pcmd->dbuser = strdup(value);
        } else if ( strcasecmp( name, "rrdfile" ) == 0 ) {
		  pcmd->rrdfile = strdup(value);
        } else if ( strcasecmp( name, "dbuser" ) == 0 ) {
		  pcmd->dbuser = strdup(value);
        } else {
	  rbuf = mkmsg("unknown option '%s' inf configuration file\n", name );
          return(rbuf);
        }
    }		
  }
  return(rbuf);  
}


/* echoconfig

   echo datastructure cmd
*/
char *
echoconfig (struct cmd *pcmd) {
  int size;
  char *s;
  static char t[MAXBUFF];

  s = mkmsg("Configuration parameters\n");
  size = strlen(s) + 1;
  
  strncat(t,s,strlen(s));
  s = mkmsg("\tpcmd->command: %d\n", pcmd->command);
  strncat(t,s, strlen(s));

  s = mkmsg("\tpcmd->argcmd: %d\n",pcmd->argcmd);
  strncat(t,s, strlen(s));

  s = mkmsg("\tpcmd->netflg: %d\n",pcmd->netflg);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->verbose: %d\n",pcmd->verbose);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->timeout: %d\n",pcmd->timeout);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->baudrate: %d\n",pcmd->baudrate);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->logfacility: %d\n",pcmd->logfacility);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->device: %s\n",pcmd->device);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->hostname: %s\n",pcmd->hostname);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->port: %s\n",pcmd->port);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->tnport: %s\n",pcmd->tnport);
  strncat(t,s, strlen(s)); 
  s = mkmsg("\tpcmd->xmlport: %s\n",pcmd->xmlport);
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



pid_t
Fork(void)
{
	pid_t	pid;

	if ( (pid = fork()) == -1) {
	  syslog(LOG_INFO,"fork error");
	  werrno = ESIG;
	  return(-1);
	}
	return(pid);
}


int
Close(int fd)
{
  if (close(fd) == -1) {
	werrno = errno;
	syslog(LOG_INFO,"Close: close error: %s",
		   strerror(werrno));
	return(-1);
  }
  return(0);
}

ssize_t	/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;	/* and call write() again */
			else
				return(-1);	/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

int
Writen(int fd, void *ptr, size_t nbytes)
{
  if (writen(fd, ptr, nbytes) != nbytes) {
	werrno = ENET;
	syslog(LOG_INFO,"Writen: writen error");
	return(-1);
  }
  return(0);
}


ssize_t
Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = read(fd, ptr, nbytes)) == -1) {
	  werrno = errno;
	  syslog(LOG_INFO,"Read: read error: %s",
			 strerror(werrno));
	  return(-1);
	}
	return(n);
}

int
Write(int fd, void *ptr, size_t nbytes)
{
  if (write(fd, ptr, nbytes) != nbytes) {
	werrno = errno;
	syslog(LOG_INFO,"Write: write error: %s",
		   strerror(werrno));
	return(-1);
  }
  return(0);
}

