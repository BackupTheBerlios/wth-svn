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
      perror("readdata: select error: EBADF");
    else if ( errno == EINTR ) 
      perror("readdata: select error: EINTR");
    else if ( errno == EINVAL ) 
      perror("readdata: select error: EINVAL");
    else
      perror("readdata: select error");
  }

  if ( err && FD_ISSET (fd, &readfs)) {

    if ((err = read (fd, &rbyte, 1)) < 0)  { 
      printf ("reading device failed \n");
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
shuffdat( unsigned char *cdat, unsigned char *rdat, int len) {
  int i;

  cdat[0] = 0xff;
  cdat[1] = 0xff;

  for ( i = 0; i < len-2; i++) {
    cdat[i+2] = rdat[i];
  }

  return(0);
}

int 
wmr9x8rd( int rfd, unsigned char *data, int *ndat) {
  int err = -1;
  int sync, cnt;
  unsigned char schr;
  unsigned char rdat[TBUFF+1];
  unsigned char cdat[TBUFF+1];

  sync = 0; cnt = 0;
  for (;;) {
    err = getschar( rfd, &schr);
    if ( err == 1) {
      printf("readdata: err: %d schr : %02x\n", err, schr);
      rdat[cnt] = schr;
      cnt++;
    } else {
      printf("readdata: could not read 1 char err: %d\n", err);
    }

    if ( schr == 0xff) {
      sync++;
    }

    if ( sync == 2) {
      printf("readdata: header sync received\n");
      shuffdat( cdat, rdat, cnt);
      printf("raw data\n");
      echodata( rdat, cnt);
      printf("corrected data\n");
      echodata( cdat, cnt);
      sync = 0; cnt = 0;
    }
  }

  return(err); 
}



void *
wmr9x8_hd( void *arg) {
  int rfd, tset, i;
  int err, mdat;
  static unsigned char data[TBUFF+1];
  struct termios tp, op;

  rfd = open("/dev/ttyd0", O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
  if (rfd == -1)
  {
    syslog(LOG_ALERT, "wmr9x8_hd: couldn't open /dev/ttyd0");
  } else {
    fcntl(rfd, F_SETFL, 0);
  }

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
  err = wmr9x8rd( rfd, data, &mdat);

  /* reset terminal attributes */
  tcsetattr(rfd, TCSANOW, &op);
  tcgetattr(rfd, &tp);

  close(rfd);
  printf("close\t\t: errno: %d\n", errno);
}
