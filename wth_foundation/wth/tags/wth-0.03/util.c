/* util.c


   utility functions

   $Id: util.c,v 0.4 2001/09/14 15:40:48 jahns Exp jahns $
   $Revision: 0.4 $

   Copyright (C) 2000-2001 Volker Jahns <Volker.Jahns@thalreit.de>

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

/* 
   getbits

   transfrom bytes of weather station data to BCD, otherwise picking single
   or groups of bits.
   Kerninghan, Ritchie, The C Programming Language, p49.
 */
unsigned
getbits(unsigned x, int p, int n) {
  return ( x>>(p+1-n)) & ~(~0 <<n);
}


/*

   digitize - stores digits of integer value in seperate 
   char variables

   implementation valid for unit, tens, hundreds and thousands
   only
*/
/* int digitize(int val, char *tdigit, char *hdigit, 
             char *tendigit, char *udigit) {

    *udigit = val % 10;
    val = val / 10;
    *tendigit = val % 10;
    val = val /10;
    *hdigit = val % 10;
    val = val / 10;
    *tdigit = val % 10;

    return(0);
}
*/

/* echodata
  
   print out raw data frame reveived from weather station 

*/
int
echodata(unsigned char *data, int mdat) {
    int i;
    char frame[255] = "";
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


/* wstrlen

   Kerninghan, Ritchie, The C Programming Language, p103

*/
int
wstrlen (char *s) {
    char *p = s;

    while ( *p != ETX ) {
	p++;
    }
    return p + 1 - s;
    
}


/* mkmsg

   man snprintf (LINUX) sample code modified
   
*/
char *
mkmsg(const char *fmt, ...) {
   int n, size = 100;
   char *p;
   va_list ap;
   if ((p = malloc (size)) == NULL)
      return NULL;
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
      fprintf(stderr,"Usage: wthc [Options]\n"
                     "where options include:\n"
                     "\t-c <command>\texecute command\n"
                     "\t\t0\tPoll DCF Time\n"
                     "\t\t1\tRequest dataset\n"
                     "\t\t2\tSelect next dataset\n"
                     "\t\t3\tActivate 9 temperature sensors\n"
                     "\t\t4\tActivate 16 temperature sensors\n"
                     "\t\t5\tRequest status\n"
                     "\t\t6\tSet interval time,\n" 
                     "\t\t\t\trequires extra parameter specified by option -i\n"
	             "\t\t12\tRequest all available data recursively\n"
                     "\t-i <interval>\tspecifies interval time\n"
		     "\t\tpermissible values for the interval lie\n"
		     "\t\twithin the range from 1 to 60 minutes\n"
                     "\t-h <hostname>\tconnect to <hostname/ipaddress>\n"
                     "\t-p <portnumber>\tuse portnumber at remote host\n"
                     "\t\tfor connection\n"
                     "\t-s\t\tuse local serial connection\n");

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
  int size = MAXBUFF;
  int ival;
  FILE *cfg;
  char line[BUFFSIZE];
  char *name;
  char *value;
  char *cp, *cp2;
  char *rbuf = "";

  rbuf = (char *) malloc(size);
  if ( rbuf == NULL)
	return NULL;
  
  if ( ( cfg = fopen("/etc/wth/wth.conf","r")) == NULL )
	rbuf = mkmsg("No configuration file found\n");
  else
	rbuf = mkmsg("Using config file: /etc/wth/wth.conf");
  
  if ( ( cfg = fopen("/usr/local/etc/wth/wth.conf","r")) == NULL ) {
	rbuf = mkmsg("No configuration file found, using default parameters\n");
	return(rbuf);
  }
  else
	rbuf = mkmsg("Using config file: /usr/local/etc/wth/wth.conf\n");

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
  char *t;

  s = mkmsg("Configuration parameters\n");
  size = strlen(s) + 1;
  
  /* t = strdup(s); */
  /* printf("echoconfig: t: \"%s\"\n", t); */
  
  
  t = (char *) malloc(strlen(s) + 1);
  if ( t == NULL)
	return NULL;
  strcpy(t,s);
    
  s = mkmsg("\tpcmd->command: %d\n", pcmd->command);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);

  s = mkmsg("\tpcmd->argcmd: %d\n",pcmd->argcmd);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);
  s = mkmsg("\tpcmd->netflg: %d\n",pcmd->netflg);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);
  s = mkmsg("\tpcmd->verbose: %d\n",pcmd->verbose);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);
  s = mkmsg("\tpcmd->timeout: %d\n",pcmd->timeout);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);
  s = mkmsg("\tpcmd->baudrate: %d\n",pcmd->baudrate);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);
  s = mkmsg("\tpcmd->logfacility: %d\n",pcmd->logfacility);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);
  s = mkmsg("\tpcmd->device: %s\n",pcmd->device);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);
  s = mkmsg("\tpcmd->hostname: %s\n",pcmd->hostname);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);
  s = mkmsg("\tpcmd->port: %s\n",pcmd->port);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);
  s = mkmsg("\tpcmd->tnport: %s\n",pcmd->tnport);
  size = size + strlen(s) +1;
  if ((t = realloc(t,size)) == NULL )
    return NULL;
  strcat(t,s);
 

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

ssize_t						/* Write "n" bytes to a descriptor. */
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
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
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

