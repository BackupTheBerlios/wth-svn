/* pcwsr.c

   serial line communication w/ PC weathersensor receiver

   $Id: pcwsr.c,v 1.2 2001/11/16 10:46:01 jahns Exp jahns $
   $Revision: 1.2 $

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
#include <time.h>
#include <string.h>

#define SDEVICE "/dev/ttyS0"
#define BAUDRATE B19200
#define MAXBUFF 1024
#define TIMEOUT 200
#define FRAMELEN 8
#define DEBUG 1



/*  initserial

  opens serial port for communication
  serial port settings:
     19200, 8, odd parity,  2stop

*/
static int 
initserial (struct termios *newtio, struct termios *oldtio, char *sdevice ) {
  int pfd;

  /* open the device to be non-blocking (read will return immediatly) */
  pfd = open(sdevice, O_RDWR| O_NOCTTY| O_NDELAY| O_NONBLOCK ); 
  
  /* Make the file descriptor asynchronous (the manual page says only 
     O_APPEND and O_NONBLOCK, will work with F_SETFL...) */
  fcntl(pfd, F_SETFL, FASYNC); 
  
  /* save current port settings */
  tcgetattr(pfd,oldtio);
  
  /* prefill port settings */
  tcgetattr(pfd,newtio);

  /* set communication link parameters */
  cfsetispeed(newtio, BAUDRATE);  
  cfsetospeed(newtio, BAUDRATE);	

  
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

  tcsetattr(pfd,TCSANOW,newtio);

  return(pfd);
}


/*
   closeserial

   restore the old port settings
   close port

*/
static int closeserial( int fd, struct termios *oldtio) {

    /* restore old port settings */
    tcsetattr(fd,TCSANOW,oldtio);
	
    /* close serial port */
    close(fd);
    return(0);
}



char
*sname(int snum)
{
  static char *name[] = {
    "outdoor", "outdoor", 
    "rain", "wind", 
    "indoor", "Helligkeit", 
    "Pyranometer", "undef"
  };
  return ( snum < 0 || snum >6 ) ? name[7] : name[snum];
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
  unsigned char *rbuf;
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
          return(-1);
        case 0:
          printf("select\t\t: Timeout\n");
          return(-1);
        default:
          
          /* loop until err = 0 */
          while ( ( rbuf[err-1] != 3) && ( err != 8)) {
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


/* 

   mini routine to read serial line 

*/
int
readserial( int fd, char *data) {
  int err;

  err = read(fd, data, FRAMELEN);
  return(err);
}



/*  
   get raw data from serial interface
*/
int 
main( int argc, char **argv) {
    int i, fd, err;              /* filedescriptor serial port */
    int hi, lo, dummy;
    int mask = 0x7f;
    int shift = 0x7;
    int styp, saddr, sver;       /* sensor typ, address and version */
    float t;
    int h, p;
    int debug;

    struct termios newtio,oldtio; /* termios structures to set comm parameters */
    unsigned char data[MAXBUFF];
    char clk[MAXBUFF];
    char *name;  /* sensor typ spec */

    time_t mtime;
    struct tm *ctm;


    if (argc < 2) {
      printf("Usage: $0 <serial device>\n");
      exit(-1);
    }

    tzset();                                   /* setting timezone */
    fd = initserial(&newtio, &oldtio, argv[1]); /* serial initialization */ 
    debug = 0;

    for ( ;; ) {
      /* read weather station resonse from serial port */
      /*  err = readdata(fd);*/
      err = readserial(fd, data);
      time(&mtime);
      ctm = gmtime(&mtime);
      strftime(clk, sizeof(clk), "%c", ctm);

      if (debug) {
	printf("%s : err: %d  |", clk, err);
	for ( i = 0; i<8; i++) {
	  printf("%x|", data[i]);
	}
	printf("\n");
      }

      /* figure sensor type and address */
      dummy = data[1] & mask;
      styp  = dummy & 0xf0;
      styp  = styp >> 4;
      saddr = dummy & 0xf;
      name = sname(styp);

      sver = 0x12;
      if ( ( styp < 0x5 ) && ( saddr < 0x8)) {
	  sver = 0x11;
      }

      if (debug) {
	printf("dummy :%x, styp :%x, saddr:%x\n", dummy, styp, saddr);      
	printf("sname: %s, styp  :%x, sver :%x\n", name, styp, sver);
      }

      /* handle different type of sensors */
      /* outdoor (temperature only) */
      if ( styp == 0x0 ) {
	/* temperature */
	hi = data[2] & mask ;
        hi = hi << shift;
	lo = data[3] & mask;
	t  = ( hi + lo ) / 10.0 ;

	if (debug) {
	  printf("Temperature\n");
	  printf("W1: %x, W2: %x\n", data[2], data[3]);
	  printf("hi : %x, lo: %x, Temp: %.1f\n\n", hi, lo, t);
	  printf("Humidity\n");
	  printf("W3: %x\n", data[4]);
	  printf("Hum: %d\n\n", h);
	}

        printf("%s | %8s (address 0x0%x) | T[°C] %.1f\n",
	       clk, name, saddr, t);
      }
      /* outdoor (temperature, humidity) */
      else if ( styp == 0x1 ) {
	/* temperature */
	hi = data[2] & mask ;
        hi = hi << shift;
	lo = data[3] & mask;
	t  = ( hi + lo ) / 10.0 ;

	/* humidity */
	h = data[4] & mask;

	if (debug) {
	  printf("Temperature\n");
	  printf("W1: %x, W2: %x\n", data[2], data[3]);
	  printf("hi : %x, lo: %x, Temp: %.1f\n\n", hi, lo, t);
	  printf("Humidity\n");
	  printf("W3: %x\n", data[4]);
	  printf("Hum: %d\n\n", h);
	}

        printf("%s | %8s (address 0x0%x) | T[°C] %.1f | H[%% rel.hum] %d\n",
	       clk, name, saddr, t, h);

      } 
      /* indoor (temperature, humidity, pressure) */
      else if ( styp == 0x4) {  
	/* temperature */
	hi = data[2] & mask;
        hi = hi << shift;
	lo = data[3] & mask;
	t  = ( hi + lo ) / 10.0 ;

	/* humidity */
	h = data[4] & mask;

	/* pressure */
	dummy  = data[5] & mask;
	hi = dummy << shift;
	lo = data[6] & mask;
	p  = hi + lo;

        if (debug) {
	  printf("Temperature\n");
	  printf("W1: %x, W2: %x\n", data[2], data[3]);
	  printf("hi : %x, lo: %x, Temp: %.1f\n\n", hi, lo, t);
	  printf("Humidity\n");
	  printf("W3: %x\n", data[4]);
	  printf("Hum: %d\n\n", h);
	  printf("Pressure\n");
	  printf("W4: %x, W5: %x\n", data[5], data[6]);
	  printf("dummy: %x, hi : %x, lo: %x, Press: %d\n", dummy, hi, lo, p);
	}

        printf("%s | %8s (address 0x0%x) | T[°C] %.1f | H[%% rel.hum] %d | P[hPa] %d\n",
	       clk, name, saddr, t, h, p);

      }
    }
    /* leave serial line in good state */ 
    closeserial(fd, &oldtio); 
    return(0);
}









