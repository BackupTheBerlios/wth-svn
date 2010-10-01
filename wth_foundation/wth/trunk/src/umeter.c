/* 
  umeter.c

  ULTIMETER weatherstation handler implemented as POSIX thread

  $Id$
  $Revision$

  Copyright (C) 2010 Volker Jahns <volker@thalreit.de>

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
  umeter_hd

  POSIX thread to handle ULTIMETER weatherstation

  opens port
  call to data reading subroutine

*/
void *
umeter_hd( void *arg) {
  int pfd, err;
  struct termios tp, op;

  syslog( LOG_DEBUG, "umeter_hd: start of execution");

  /* initialize serial port */
  if ( initumeter(&pfd, &tp, &op) == -1 )
    return( ( void *) &failure);

  umeterstation.status.is_present = 1;

  /* read ULTIMETER weatherstation */
  err = umeter_rd( pfd);

  closeumeter(pfd, &op);

  return( ( void *) &success);
}

/*
  umeter_rd

  reading data records from serial port
  data acquisition to database

*/
int 
umeter_rd( int rfd) {
  int err = -1;
  int ndat;
  static unsigned char data[255];

  for (;;) {
    ndat = read(rfd,data,255); 
    data[ndat-1]=0;
    if ( ndat == 51) 
      printf("data: '%s' : %d\n", data, ndat-1);
  }
  return(err); 
}

/*  initumeter

  opens serial port for communication
  
  serial port settings:
  ULTIMETER 2100 (and similar stations)
     2400, 8 data bits, no parity, 1 stop
*/
int 
initumeter (int *pfd, struct termios *newtio,
	    struct termios *oldtio) 
{

  *pfd = open(umeterstation.config.device, O_RDWR| O_NOCTTY );
  if ( pfd <0) {
    werrno = errno;
    syslog(LOG_INFO, "initumeter: error opening serial device : %s",
	   strerror(werrno));
    return(-1);
  }

  tcgetattr(*pfd,oldtio); /* save current serial port settings */
  memset(newtio, 0, sizeof(*newtio)); 
        
  newtio->c_cflag =  CS8 | CLOCAL | CREAD;
  newtio->c_iflag = IGNPAR | ICRNL;
  newtio->c_oflag = 0;
  newtio->c_lflag = ICANON;

  cfsetospeed(newtio,B2400);            // 2400 baud
  cfsetispeed(newtio,B2400);            // 2400 baud

  tcsetattr(*pfd,TCSANOW,newtio);

  return(0);
}

/*
   closeumeter

   restore the old port settings
   lower DTR on serial line

*/
int closeumeter( int fd, struct termios *oldtio) {

    /* restore old port settings */
    /* takes approx 3 sec to reopen fd under FreeBSD
       if tcsetattr is called to restore */
    if ( tcsetattr(fd,TCSANOW,oldtio) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closeumeter: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
	
    /* close serial port */
    if ( close(fd) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closeumeter: error close: %s",
			 strerror(werrno));
	  return(-1);	
    }
    return(0);
}

