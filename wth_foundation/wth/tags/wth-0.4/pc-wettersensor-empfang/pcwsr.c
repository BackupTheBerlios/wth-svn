/* pcwsr.c

   serial line communication w/ PC weathersensor receiver

   $Id$
   $Revision$

   Copyright (C) 2001 Volker Jahns <Volker.Jahns@thalreit.de>

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

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

#define SDEVICE "/dev/ttyd0"
#define BAUDRATE 19200
#define MAXBUFF 1024
#define TIMEOUT 200

/*  initserial

  opens serial port for communication
  serial port settings:
     19200, 8, odd parity,  2stop

*/
static int 
initserial (struct termios *newtio, struct termios *oldtio ) {
  int i, itio, pfd;

  /* open the device to be non-blocking (read will return immediatly) */
  pfd = open(SDEVICE, O_RDWR| O_NOCTTY| O_NDELAY| O_NONBLOCK ); 
  
  /* Make the file descriptor asynchronous (the manual page says only 
     O_APPEND and O_NONBLOCK, will work with F_SETFL...) */
  fcntl(pfd, F_SETFL, FASYNC); 
  
  /* save current port settings */
  tcgetattr(pfd,oldtio);
  
  /* prefill port settings */
  tcgetattr(pfd,newtio);

  /* set communication link parameters */
  
  cfsetispeed(newtio, 19200);  
  cfsetospeed(newtio, 19200);	

  
/*
  newtio->c_iflag = 0;
  newtio->c_oflag = 0;
  newtio->c_cflag = 0;
  newtio->c_lflag = 0;

  for ( i = 0; i < NCCS; i++) {
	newtio->c_cc[i] = 0;
  }
*/

  
  /* newtio->c_iflag &= (IXON | IXOFF | IXANY);   */
  newtio->c_cflag |= PARENB;
  newtio->c_cflag |= PARODD;
  newtio->c_cflag |= CLOCAL;
  newtio->c_cflag |= CREAD;
  newtio->c_cflag |= CS8;
  newtio->c_cflag |= CSTOPB;
  newtio->c_cflag |= CSIZE; 
  
  newtio->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  newtio->c_cc[VMIN]=1;
  newtio->c_cc[VTIME]=0;

  tcsetattr(pfd,TCSANOW,newtio);

  return(pfd);
}


/*
   closeserial

   restore the old port settings
   close port

*/
static int closeserial( int fd, struct termios *oldtio) {
    int tset;

    /* restore old port settings */
    tcsetattr(fd,TCSANOW,oldtio);
	
    /* close serial port */
    close(fd);
    return(0);
}



/*
  readdata

  reading data from serial port

*/
static int 
readdata (int rfd)
{
  int err, i, n;
  int maxfd;
  int loop = 1;
  fd_set readfs;
  unsigned char rbuf[MAXBUFF];
  struct timeval tv;
  
  err = -1;
  maxfd = rfd +1;

  
  for ( i = 0; i < 254; i++ ){
      rbuf[i] = 0;
  }

  /* loop until input is available */
  while ( loop) {
        
        FD_ZERO(&readfs);
        FD_SET(rfd, &readfs);

        tv.tv_sec  = TIMEOUT;
        /* tv.tv_usec = 0; */
        
        n = select(maxfd, &readfs, NULL, NULL, &tv);
        switch (n) {
        case -1:
          perror("select");
          exit(1);
        case 0:
          printf("select\t\t: Timeout\n");
          exit(1);
        default:
          /* loop until err = 0 */
          while ( ( rbuf[err-1] != 3) && ( err != 0)) {
                err = read(rfd, rbuf, MAXBUFF);
                perror("read returned");
                printf("read\t\t: errno: %d\n", errno);

                printf("read\t\t: err %d: rbuf: |", err);
                for ( i = 0; i < err; i++) {
                  printf("%d|", rbuf[i]);
                }
                printf("\n");
                loop=0;
          }
          break;
        }
  }
  return(0);
}


int
input_loop (int rfd)
{
  int err,i;
  char rbuf[MAXBUFF];

  err = -1;
  for ( i = 0; i < 254; i++ ){
      rbuf[i] = 0;
  }

  /* loop until err = 0 */
/*
  while ( ( rbuf[err-1] != 3) && ( err != 0)) {
    err = read(rfd, rbuf, MAXBUFF);
    printf("read\t\t: errno: %d\n", errno);
    perror("read returned");
    printf("read\t\t: err %d: rbuf: |", err);
    for ( i = 0; i < err; i++) {
        printf("%d|", rbuf[i]);
    }
    printf("\n");
  }
*/
  while(1) {

  }
  return(0);
}


/*  

   get raw data from serial interface
*/
int 
main (int argc, char **argv) {
    int fd, ndat, err;              /* filedescriptor serial port */
    struct termios newtio,oldtio;   /* termios structures to set 
                                     comm parameters */
    unsigned char *data;

    /* serial initialization */
    fd = initserial(&newtio, &oldtio);

    /* read weather station resonse from serial port */
    err = input_loop(fd); 
    
    /* leave serial line in good state */ 
    /* closeserial(fd, &oldtio); */

    return(0);
}

