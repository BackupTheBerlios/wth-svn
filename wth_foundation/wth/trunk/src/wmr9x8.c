/* 
  wmr9x8.c

  wmr9x8 handler implemented as POSIX thread

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

/*  initwmr9x8

  opens serial port for communication
  
  serial port settings:
  WMR928 (and similar stations)
     9600, 8, Even Parity,  2Stop
     raise RTS voltage


*/
int 
initwmr9x8 (int *pfd, struct termios *newtio,
	    struct termios *oldtio) 
{
  int i, itio;

  /* open the device to be non-blocking (read will return immediatly) */

  *pfd = open(wmr9x8station.config.device, O_RDWR| O_NOCTTY| O_NDELAY| O_NONBLOCK );
  if ( *pfd <0) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error opening serial device : %s",
	   strerror(werrno));
    return(-1);
  }

  /* is this asynchronous? probably not */
  if ( fcntl(*pfd, F_SETFL, 0) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error fcntl: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* save current port settings */
  if ( tcgetattr(*pfd,oldtio) == -1 ) {  
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error tcgetattr oldtio: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* prefill port settings */
  if ( tcgetattr(*pfd,newtio) == -1 ) { 
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error tcgetattr newtio: %s",
	   strerror(werrno));
    return(-1);	
  }
  
  /* set communication link parameters */
  if ( cfsetispeed(newtio, B9600) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error cfsetispeed: %s",
	   strerror(werrno));
    return(-1);	
  }
  
  if ( cfsetospeed(newtio, B9600) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error cfsetospeed: %s",
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
    syslog(LOG_INFO, "initwmr9x8: error tcsetattr: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* lower DTR and RTS on serial line */
  if ( ioctl(*pfd, TIOCMGET, &itio) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }
  itio &= ~TIOCM_DTR;
  itio &= ~TIOCM_RTS;
  if ( ioctl(*pfd, TIOCMSET, &itio) == -1 ) {
    errno = errno;
    syslog(LOG_INFO, "initwmr9x8: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* raise RTS */
  itio |= TIOCM_RTS;
  if ( ioctl(*pfd, TIOCMSET, &itio) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }
  return(0);
}

/*
   closewmr9x8

   restore the old port settings
   lower DTR on serial line

*/
int closewmr9x8( int fd, struct termios *oldtio) {
    int tset;

    /* lower RTS on serial line */
    if ( ioctl(fd, TIOCMGET, &tset) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closewmr9x8: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
    tset &= ~TIOCM_RTS;
    if ( ioctl(fd, TIOCMSET, &tset) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "closewmr9x8: error ioctl: %s",
	     strerror(werrno));
      return(-1);	
    }

    /* restore old port settings */
    /* takes approx 3 sec to reopen fd under FreeBSD
       if tcsetattr is called to restore */
    if ( tcsetattr(fd,TCSANOW,oldtio) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closewmr9x8: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
	
    /* close serial port */
    if ( close(fd) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closewmr9x8: error close: %s",
			 strerror(werrno));
	  return(-1);	
    }
    return(0);
}


int 
getschar (int fd, unsigned char *schar) 
{
  int err = -1;
  unsigned char rbyte;
  int maxfd = fd + 1;
  fd_set readfs;
  struct timeval tv;

  tv.tv_sec  = 7;
  tv.tv_usec = 0;

  FD_ZERO(&readfs);
  FD_SET(fd, &readfs);

  err = select(maxfd, &readfs, NULL, NULL, &tv);

  if ( err < 0 ) {
    if ( errno == EBADF ) 
      syslog(LOG_ALERT,"getschar: select error: EBADF");
    else if ( errno == EINTR ) 
      syslog(LOG_ALERT,"getschar: select error: EINTR");
    else if ( errno == EINVAL ) 
      syslog(LOG_ALERT,"getschar: select error: EINVAL");
    else
      syslog(LOG_ALERT,"getschar: select error");
  }

  if ( err && FD_ISSET (fd, &readfs)) {

    if ((err = read (fd, &rbyte, 1)) < 0)  { 
      syslog (LOG_ALERT,"getschar: reading device failed \n");
      return (err);
    }
    else if (err == 1) {
      *schar = rbyte;
      return( err);
    }
  }
  return( err);
}

int
shuffdat( unsigned char *data, int ndat) {
  int i;
  unsigned char tdat[TBUFF+1]; 
  tdat[0] = 0xff;
  tdat[1] = 0xff;

  for ( i = 0; i < ndat-2; i++) {
    tdat[i+2] = data[i];
  }

  for ( i = 0; i < ndat; i++) 
    data[i] = tdat[i];

  return(0);
}

int
wmr9x8dac( unsigned char *data, int ndat) {
  int err;

  return(err);
}


int 
wmr9x8rd( int rfd) {
  int err = -1;
  int sync, ndat;
  unsigned char schr;
  unsigned char data[TBUFF+1];

  sync = 0; ndat = 0;
  for (;;) {
    err = getschar( rfd, &schr);
    if ( err == 1) {
      printf("wmr9x8rd: err: %d schr : %02x\n", err, schr);
      data[ndat] = schr;
      ndat++;
    } else {
      printf("wmr9x8rd: could not read 1 char err: %d\n", err);
    }

    if ( schr == 0xff) {
      sync++;
    }

    if ( sync == 2) {
      printf("wmr9x8rd: header sync received\n");
      //shuffdat( cdat, rdat, cnt);
      printf("raw data\n");
      echodata( data, ndat);

      printf("corrected data\n");
      shuffdat( &data, ndat);
      echodata( data, ndat);
      sync = 0; ndat = 0;
    }
  }

  return(err); 
}



void *
wmr9x8_hd( void *arg) {
  int rfd, tset, i;
  int err, mdat;
  struct termios tp, op;

  syslog( LOG_DEBUG, "wmr9x8_hd: start of execution");

  /* initialize serial port */
  if ( initwmr9x8(&rfd, &tp, &op) == -1 )
    return(-1);
  
  wmr9x8station.status.is_present = 1;

  /*
  rfd = open("/dev/ttyd0", O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
  if (rfd == -1)
  {
    syslog(LOG_ALERT, "wmr9x8_hd: couldn't open /dev/ttyd0");
  } else {
    fcntl(rfd, F_SETFL, 0);
  }
  */

  /* get terminal attributes */
  tcgetattr(rfd, &op);
  tcgetattr(rfd, &tp);

  /* set terminal attr */
  cfsetispeed(&tp, 9600);
  cfsetospeed(&tp, 9600);

  tp.c_iflag = 0;
  tp.c_oflag = 0;
  tp.c_cflag = 0;
  tp.c_lflag = 0;

  for ( i = 0; i < NCCS; i++) {
	tp.c_cc[i] = 0;
  }

  tp.c_iflag &= (IXON | IXOFF | IXANY);
  
  tp.c_cflag |= PARENB;
  tp.c_cflag &= ~PARODD;
  tp.c_cflag |= CLOCAL;
  tp.c_cflag |= CREAD;
  tp.c_cflag |= CS8;
  tp.c_cflag |= CSTOPB;
  tp.c_cflag |= CSIZE; 
  
  tp.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  tp.c_cc[VMIN]=0;
  tp.c_cc[VTIME]=1;

  tcsetattr(rfd, TCSANOW, &tp);
  tcgetattr(rfd, &tp);

  /* raise DTR and RTS */
  tset |= TIOCM_DTR;
  tset |= TIOCM_RTS;
  ioctl(rfd, TIOCMSET, &tset);

  /* read WMR 9x8 weatherstation */
  err = wmr9x8rd( rfd);

  /* reset terminal attributes */
  tcsetattr(rfd, TCSANOW, &op);
  tcgetattr(rfd, &tp);

  close(rfd);
  printf("close\t\t: errno: %d\n", errno);
}
