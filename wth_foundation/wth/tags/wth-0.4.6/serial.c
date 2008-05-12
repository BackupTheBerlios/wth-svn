/* serial.c

   serial line communication routine for WS2000 weatherstation

   $Id$
   $Revision$

   Copyright (C) 2000-2001,2005,2007 Volker Jahns <volker@thalreit.de>

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
bitstat( int byte, char *s_reg  ) 
{
  int x;
  struct timezone tz;
  struct timeval  tv;

  gettimeofday(&tv, &tz);
  printf("%5s | %lu.%lu : #", s_reg, tv.tv_sec, tv.tv_usec);
  for( x = 7; x > -1; x-- )
    printf( "%i", (byte & 1 << x ) > 0 ? 1 : 0 );
  printf( "b\n" );
  return(0);
}


/*  initserial

  opens serial port for communication
  
  serial port settings:
  WS2000 (and similar stations)
     9600, 8, Even Parity,  2Stop
     lower RTS and raise DTR voltage

  PCWSR (PC Weatherstation receiver by ELV)
     19200 BAUD, 8, Odd Parity, 2 Stopp

*/
int 
initserial (int *pfd, struct termios *newtio,
	    struct termios *oldtio, struct cmd *pcmd) 
{
  int i, itio;

  if ( strcmp( pcmd->wstype, "WS2000") == 0 ) /* WS2000 */
    { 
    /* open the device to be non-blocking (read will return immediatly) */

    *pfd = open(pcmd->device, O_RDWR| O_NOCTTY| O_NDELAY| O_NONBLOCK );
    if ( *pfd <0) {
      werrno = errno;
      syslog(LOG_INFO, "initserial: error opening serial device : %s",
	     strerror(werrno));
      return(-1);
    }

    /* is this asynchronous? probably not */
    if ( fcntl(*pfd, F_SETFL, 0) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "initserial: error fcntl: %s",
	     strerror(werrno));
      return(-1);	
    }

    /* save current port settings */
    if ( tcgetattr(*pfd,oldtio) == -1 ) {  
      werrno = errno;
      syslog(LOG_INFO, "initserial: error tcgetattr oldtio: %s",
	     strerror(werrno));
      return(-1);	
    }

    /* prefill port settings */
    if ( tcgetattr(*pfd,newtio) == -1 ) { 
      werrno = errno;
      syslog(LOG_INFO, "initserial: error tcgetattr newtio: %s",
	     strerror(werrno));
      return(-1);	
    }
  
    /* set communication link parameters */
    if ( cfsetispeed(newtio, pcmd->baudrate) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "initserial: error cfsetispeed: %s",
	     strerror(werrno));
      return(-1);	
    }
  
    if ( cfsetospeed(newtio, pcmd->baudrate) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "initserial: error cfsetospeed: %s",
	     strerror(werrno));
      return(-1);	
    }


    newtio->c_iflag = 0;
    newtio->c_oflag = 0;
    newtio->c_cflag = 0;
    newtio->c_lflag = 0;

    for ( i = 0; i < NCCS; i++) {
      newtio->c_cc[i] = 0;
    }
  
  
    /* newtio->c_iflag &= (IXON | IXOFF | IXANY);  */  
    newtio->c_cflag |= PARENB;
    newtio->c_cflag &= ~PARODD;
    newtio->c_cflag |= CLOCAL;
    newtio->c_cflag |= CREAD;
    newtio->c_cflag |= CS8;
    newtio->c_cflag |= CSTOPB;
    newtio->c_cflag |= CSIZE; 
  
    /* newtio->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); */
    newtio->c_cc[VMIN]=0;
    newtio->c_cc[VTIME]=10;

    if ( tcsetattr(*pfd,TCSANOW,newtio) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "initserial: error tcsetattr: %s",
	     strerror(werrno));
      return(-1);	
    }

    /* lower DTR and RTS on serial line */
    if ( ioctl(*pfd, TIOCMGET, &itio) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "initserial: error ioctl: %s",
	     strerror(werrno));
      return(-1);	
    }
    itio &= ~TIOCM_DTR;
    itio &= ~TIOCM_RTS;
    if ( ioctl(*pfd, TIOCMSET, &itio) == -1 ) {
      errno = errno;
      syslog(LOG_INFO, "initserial: error ioctl: %s",
	     strerror(werrno));
      return(-1);	
    }

    /* raise DTR */
    itio |= TIOCM_DTR;
    if ( ioctl(*pfd, TIOCMSET, &itio) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "initserial: error ioctl: %s",
	     strerror(werrno));
      return(-1);	
    }

  } /* WS2000 */
  else if ( strcmp(pcmd->wstype, "PCWSR" ) == 0) /* PCWSR */
    {
      /* open the device to be non-blocking (read will return immediatly) */
      *pfd = open( pcmd->device, O_RDWR| O_NOCTTY| O_NDELAY| O_NONBLOCK);
      if ( *pfd <0) {
	werrno = errno;
	syslog(LOG_INFO, "initserial: error opening serial device : %s",
	       strerror(werrno));
	return(-1);
      }

      /* async comm */
      fcntl(*pfd, F_SETFL, FASYNC);
 
      /* save current port settings */
      tcgetattr(*pfd,oldtio);
  
      /* prefill port settings */
      tcgetattr(*pfd,newtio);

      /* set communication link parameters */
      cfsetispeed(newtio, pcmd->baudrate);  
      cfsetospeed(newtio, pcmd->baudrate);	

  
      newtio->c_cflag |= PARENB;
      newtio->c_cflag |= PARODD;
      newtio->c_cflag |= CLOCAL;
      newtio->c_cflag |= CREAD;
      newtio->c_cflag |= CS8;
      newtio->c_cflag |= CSTOPB;
      newtio->c_cflag |= CSIZE; 
  
      newtio->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
      newtio->c_oflag &= ~OPOST;

      newtio->c_cc[VMIN]=1;
      newtio->c_cc[VTIME]=5;

      tcsetattr(*pfd,TCSANOW,newtio);

    } 
  else {
    werrno = errno;
    syslog(LOG_INFO, "initserial: unknown WSTYPE %s",
	   pcmd->wstype);
    return(-1);	

  }
  return(0);
}


/*
   closeserial

   restore the old port settings
   lower DTR on serial line

*/
int closeserial( int fd, struct termios *oldtio) {
    int tset;

    /* lower DTR on serial line */
    if ( ioctl(fd, TIOCMGET, &tset) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closeserial: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
    tset &= ~TIOCM_DTR;
    if ( ioctl(fd, TIOCMSET, &tset) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "closeserial: error ioctl: %s",
	     strerror(werrno));
      return(-1);	
    }

    /* restore old port settings */
    /* takes approx 3 sec to reopen fd under FreeBSD
       if tcsetattr is called to restore */
    if ( tcsetattr(fd,TCSANOW,oldtio) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closeserial: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
	
    /* close serial port */
    if ( close(fd) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closeserial: error close: %s",
			 strerror(werrno));
	  return(-1);	
    }
    return(0);
}



/*
  readdata

  reading data from serial port
  signal SIGIO is sent by the weather station when data are available
  huh - the weather station sends its data frame in chunks of several bytes   

*/
int 
readdata (int fd, unsigned char *data, int *ndat, 
  struct cmd *pcmd) 
{
  int i,n;
  int err;
  unsigned char rbuf[MAXBUFF];
  int maxfd;
  int loop = 1;
  fd_set readfs;
  struct timeval tv;

  *ndat = 0;
	
  err = -1;
  maxfd = fd +1;
  
  /* loop until input is available */
  while ( loop) {
	
	FD_ZERO(&readfs);
	FD_SET(fd, &readfs);

	tv.tv_sec  = TIMEOUT;
	tv.tv_usec = 0;
	
	if ( strcmp( pcmd->wstype, "WS2000") == 0 ) /* WS2000 */
	  n = select(maxfd, &readfs, NULL, NULL, &tv);
        else if ( strcmp( pcmd->wstype, "PCWSR") == 0 ) /* PCWSR */
	  n = select(maxfd, &readfs, NULL, NULL, NULL);
        else /* return with error */
	  n = -1;
	switch (n) {
	case -1:
          if ( errno == EBADF ) 
	    syslog(LOG_INFO, "readdata: select error: EBADF");
          else if ( errno == EINTR ) 
	    syslog(LOG_INFO, "readdata: select error: EINTR");
          else if ( errno == EINVAL ) 
	    syslog(LOG_INFO, "readdata: select error: EINVAL");
          else
	    syslog(LOG_INFO, "readdata: select error");
          err = -1; loop = 0;
	  break;
	case 0:
	  syslog(LOG_INFO, "readdata: select: Timeout\n");
          err = -1; loop = 0;
          break;
	default:
	  /* loop until err = 0 */
	  while ( ( rbuf[err-1] != 3) && ( err != 0)) {
	    err = read(fd, rbuf, MAXBUFF);
	    for ( i = 0; i < err; i++) {
              data[*ndat + i] = rbuf[i];
	    }
            *ndat += err;
	    loop=0;
	  }
	  break;
	}
  }
  return(err);
}


/*  c
    
    returns 
    communication string of command 
    expected response message length 

    cmd : description                     : response length (bytes)
    0   : Poll DCF Time                   : 7
    1   : Request Dataset                 : 35/61 (violation of protocol spec)
    2   : Select Next Dataset             : 1
    3   : Activate 9 temperature sensors  : 1
    4   : Activate 16 temperature sensors : 1
    5   : Request status                  : 21
    6   : Set interval time               : 1
    shouldn't this go into a configuration file?
 */
Ckey *c(int n){
  static Ckey p[] = {
	{"\x01\x30\xcf\x04", 7, "Poll DCF time"},
	{"\x01\x31\xce\x04", 61, "Request dataset"},
	{"\x01\x32\xcd\x04", 1, "Select next dataset"},
	{"\x01\x33\xcc\x04", 1, "Activate 9 temperature sensors"},
	{"\x01\x34\xcb\x04", 1, "Activate 16 temperature sensors"},
	{"\x01\x35\xca\x04", 21, "Request status"},
	{"\x01\x36\x53\xc9\x04", 1, "Set interval time"},
	{"", 0, "Unused"},
	{"", 0, "Unused"},
	{"", 0, "Unused"},
	{"", 0, "Unused"},
	{"", 0, "Unused"},
	{"", 0, "Recursive data request"},
  };
  return p+n;
  
};



/* getsrd 

   get raw data from serial interface
*/
int getsrd ( unsigned char *data, int *mdat, struct cmd *pcmd) {
    int fd;                         /* filedescriptor serial port */
    char lword[5];                  /* temporary array to hold commandword */
    struct termios newtio,oldtio;   /* termios structures to set 
                                       comm parameters */

    
    if ( initserial(&fd, &newtio, &oldtio, pcmd) == -1 )
	  return(-1);

    if ( strcmp( pcmd->wstype, "WS2000") == 0 ) /* WS2000 */
      {
	/* read ETX from serial port
 
	   when the weather station is activated by setting DTR it answers
	   with an ETX char to signalize it's OK 
	*/
	if ( readdata(fd, data, mdat, pcmd) == -1 ) {
          printf("readdata problem\n");
	  closeserial(fd, &oldtio);
	  return(-1);
	}

	if ( strcpy(lword, c(pcmd->command)->word) == NULL )
	  return(-1); 
	
        /* modify command to set intervall time */
	if ( pcmd->command == 6) {
	  lword[2] = pcmd->argcmd;
	  lword[3] = lword[3] - pcmd->argcmd;
	}

	/* write to command to serial line */
	if ( write(fd, lword, sizeof(lword)) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "getsrd: write failed: %s",
		 strerror(werrno));
	  return(-1);
	}

	syslog(LOG_DEBUG, "getsrd : command written : %d\n", pcmd->command);
      }

    /* read weather station resonse from serial port */
    if ( readdata(fd, data, mdat, pcmd) == -1) {
      closeserial(fd, &oldtio);
      return(-1);
    }
    
    /* echo raw dataframe */
    if ( echodata( data, *mdat) == -1)
	  return(-1);

    closeserial(fd, &oldtio);


    syslog(LOG_DEBUG, "getsrd : Data length getsrd : %d\n", *mdat);
    return(0);
}








