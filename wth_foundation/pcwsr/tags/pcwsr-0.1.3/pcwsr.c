/* pcwsr.c

   serial line communication w/ PC weathersensor receiver

   $Id: pcwsr.c,v 1.5 2003/11/10 13:38:35 jahns Exp jahns $
   $Revision: 1.5 $

   negative temperature patch:
   Ulrich Vigenschow, vigger@t-online.de 

   Copyright (C) 2001,2002, 2003 Volker Jahns <Volker.Jahns@thalreit.de>

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
  if ( ( pfd = open( sdevice, O_RDWR| O_NOCTTY| O_NDELAY| O_NONBLOCK)) == -1 ) {
    perror("Unable to open serial device");
    exit(-1);
  } 
  
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
    "indoor", "brightness", 
    "radiation power", "undef"
  };
  return ( snum < 0 || snum >6 ) ? name[7] : name[snum];
}





/*  
   get raw data from serial interface
*/
int 
main( int argc, char *argv[]) {
    int c, i, fd, err;              /* filedescriptor serial port */
    int hi, lo, dummy;
    int mask = 0x7f;
    int shift = 0x7;
    int styp, saddr, sver;       /* sensor typ, address and version */
    float t, wspd, wdev;
    int h, p, wdir, r, b, rad, mul;
    int debug, raw;

    struct termios newtio,oldtio; /* termios structures to set comm parameters */
    unsigned char data[MAXBUFF];
    char clk[MAXBUFF];
    char buf[MAXBUFF];
    char *name;  /* sensor typ spec */

    time_t mtime;
    struct tm *ctm;



    printf("pcwsr 0.1.3 - experimental test version\n");
    printf("Copyright (C) 2002, 2003 Volker Jahns, Volker.Jahns@thalreit.de\n");
    debug = 0;
    raw   = 0;

    while ( --argc > 0 && (*++argv)[0] == '-')
      while ( c = *++argv[0] )
	switch (c) {
	case 'r':
	  raw = 1;
          printf("option: raw\n");
	  break;
	case 'd':
	  debug= 1;
          printf("option: debug\n");
	  break;
	default:
	  printf("Illegal option: %c\n", c);
	  argc = 0;
	  break;
	}

    if (argc !=1 ) {
      printf("Usage: pcwsr -r -d <serial device>\n");
      printf("\twhere <options> include:\n");
      printf("\t-r:\tdump raw data frames\n");
      printf("\t-d:\taddtl. debug output\n");
      exit(-1);
    }

  
    printf("%s\n", *argv);
    fflush(stdout);

    tzset();                                   /* setting timezone */
    fd = initserial(&newtio, &oldtio, *argv); /* serial initialization */ 

    for ( ;; ) {
      /* read weather station resonse from serial port */
      err = read(fd, data, FRAMELEN);

      time(&mtime);
      ctm = gmtime(&mtime);
      strftime(clk, sizeof(clk), "%a %b %d %Y %X", ctm);

      if (raw) {
	printf("%s : %d bytes read |", clk, err);
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
      name  = sname(styp);
 
      sver  = 0x12;
      if ( ( styp < 0x5 ) && ( saddr < 0x8)) {
	  sver = 0x11;
      }

      if (debug) {
	printf("dummy :%x, styp :%x, saddr:%x\n", dummy, styp, saddr);      
	printf("sname: %s, styp  :%x, sver :%x\n", name, styp, sver);
      }

      /* handle sensors */
      /* outdoor (temperature only) */
      if ( styp == 0x0 ) {
	/* temperature */
	hi = data[2] & mask ;
        hi = hi << 9;
	lo = data[3] & mask;
	lo = lo << 2;
	dummy = hi + lo;
	/* negative temperatures */
	if (dummy & 0x00004000)
	  dummy |= 0xffffc000 ;
	t  = dummy / 40.0 ;

	if (debug) {
	  printf("Temperature\n");
	  printf("W1: %x, W2: %x\n", data[2], data[3]);
	  printf("hi : %x, lo: %x, Temp: %.1f\n\n", hi, lo, t);
	}
        
        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (address 0x0%x) | T[°C] %.1f\n",
	    clk, name, saddr, t);
        }
      }
      /* outdoor (temperature, humidity) */
      else if ( styp == 0x1 ) {
	/* temperature */
	hi = data[2] & mask ;
        hi = hi << 9;
	lo = data[3] & mask;
	lo = lo << 2;
	dummy = hi + lo;
	/* negative temperatures */
	if (dummy & 0x00004000)
	  dummy |= 0xffffc000 ;
	t  = dummy / 40.0 ;

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

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | T[°C] %.1f | H[%% rel.hum] %d\n",
	    clk, name, sver, saddr, t, h);
        }
      } 
      /* rain */
      else if ( styp == 0x2 ) {
	hi = data[2] & 0x1f ;
        hi = hi << shift;
	lo = data[3] & mask;
	r  = hi + lo ;

	if (debug) {
	  printf("Rain\n");
	  printf("W1: %x, W2: %x\n", data[2], data[3]);
	  printf("hi : %x, lo: %x, Rain: %d\n\n", hi, lo, r);
	}

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | [Impulses] %d\n",
	    clk, name, sver, saddr, r);
        }
      } 
      /* wind */
      else if ( styp == 0x3 ) {
	/* wind speed*/
	hi = data[2] & mask ;
        hi = hi << shift;
	lo = data[3] & mask;
	wspd  = ( hi + lo ) / 10.0 ;

	/* wind deviation */
	dummy = data[4] & mask;
        wdev  = (dummy * 45 );
        wdev  = 0.5 * wdev;

	/* wind direction */
	hi = data[5] & mask ;
        hi = hi << shift;
	lo = data[6] & mask;
	wdir = hi + lo ;

	if (debug) {
	  printf("Windspeed\n");
	  printf("W1: %x, W2: %x\n", data[2], data[3]);
	  printf("hi : %x, lo: %x, Windspd: %.1f\n\n", hi, lo, wspd);
	  printf("Wind Deviation\n");
	  printf("W3: %x\n", data[4]);
	  printf("Wind deviation: %f\n\n", wdev);
	  printf("Wind Direction\n");
	  printf("W4: %x, W5: %x\n", data[5], data[6]);
	  printf("Wind direction: %d\n\n", wdir);
	}

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | Speed[km/s] %.1f | Direction [°] %d +/- %.1f\n",
	    clk, name, sver, saddr, wspd, wdir, wdev);
        }
      } 
      /* indoor (temperature, humidity, pressure) */
      else if ( styp == 0x4) {  
	/* temperature */
	hi = data[2] & mask;
        hi = hi << 9;
	lo = data[3] & mask;
	lo = lo << 2;
	dummy  = hi + lo;
	/* negative temperatures */
	if (dummy & 0x00004000)
	  dummy |= 0xffffc000 ;
	t  = dummy / 40.0 ;

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

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | T[°C] %.1f | H[%% rel.hum] %d | P[hPa] %d\n",
	    clk, name, sver, saddr, t, h, p);
        }
      }
      /* brightness */
      else if ( styp == 0x5) {  
	/* brightness */
	hi = data[2] & mask;
        hi = hi << shift;
	lo = data[3] & mask;
	b  = hi + lo ;

	/* multiplicator */
	mul = data[4] & 0x3;

        if (debug) {
	  printf("Brightness\n");
	  printf("W1: %x, W2: %x\n", data[2], data[3]);
	  printf("hi : %x, lo: %x, brightness: %d\n\n", hi, lo, b);
	  printf("Multiplicator\n");
	  printf("W3: %x\n", data[4]);
          printf("mul: %d\n", mul);
	}

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | [arb.units] %d x %d\n",
	    clk, name, sver, saddr, b, mul);
        }
      }
      /* pyranometer */
      else if ( styp == 0x6) {  
	/* pyranometer */
	hi = data[2] & mask;
        hi = hi << shift;
	lo = data[3] & mask;
	rad  = hi + lo;

	/* multiplicator */
	mul = data[4] & 0x3;

        if (debug) {
	  printf("Pyranometer\n");
	  printf("W1: %x, W2: %x\n", data[2], data[3]);
	  printf("hi : %x, lo: %x, Radiation Power: %d\n\n", hi, lo, rad);
	  printf("Multiplicator\n");
	  printf("W3: %x\n", data[4]);
	  printf("mul: %d\n\n", h);
	}

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | Radiation Power[arb.unit] %d x %d\n",
	    clk, name, sver, saddr, rad, mul);
        }
      }
      /* sensor unknown ?! */
      else {

	snprintf(buf, sizeof(buf), "%s | unknown sensor type ( possible version %x at address 0x0%x): Please report incident to: Volker.Jahns@thalreit.de\n",
		clk, sver, saddr);
      }
      if (!raw) printf("%s\n", buf);
      fflush(stdout);
    }
    /* leave serial line in good state */ 
    closeserial(fd, &oldtio); 
    return(0);
}









