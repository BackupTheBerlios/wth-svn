/* pcwsr.c

   serial line communication w/ PC weathersensor receiver

   $Id: pcwsr.c,v 1.7 2004/05/26 11:53:20 jahns Exp jahns $
   $Revision: 1.7 $

   Copyright (C) 2001-2004 Volker Jahns <Volker.Jahns@thalreit.de>

   v0.1.4: "-p" Pipeline-mode patch, etc: (December 2003)
   Martin Johnson, pcwsr@martinshouse.com

   v0.1.3: negative temperature patch:
   Ulrich Vigenschow, vigger@t-online.de

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

/*
 *  Update v0.1.4 (December 2003, Martin Johnson pcwsr@martinshouse.com) :-
 *
 *  1. Changed serial comms processing to do a little more checking, and to
 *     avoid blocking (see 2.).
 *
 *  2. Caught signals HUP and INT, and made them close down cleanly.
 *
 *  3. Changed all existing output to go to STDOUT by default,
 *     but to STDERR if "-p" is used.
 *
 *  4. Added command-line option "-p" to send "pipeline-compatible" output
 *     to STDOUT.  The output format style of "-p" is a simple comma-delimited
 *     format, which you can use like this:
 *          pcwsr -p /dev/cuaa0 | pcwsr_to_postgresql
 *     to place sensor data into a PostgreSQL database, for later web display
 *     with PHP and GD.
 *
 *     (Martin is writing pcwsr_to_postgresql!  Please don't hassle Volker
 *     about that. Email pcwsr@martinshouse.com instead.)
 *
 *     If you use -p to send data to a pipe, you will still see
 *     the normal human-readable output on STDERR.
 *
 *  5. Removed extra "\n" newlines in output.
 *     Reformatted output values from Brightness and Pyranometer sensors.
 */

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define BAUDRATE B19200
#define MAXBUFF 1024
#define TIMEOUT 200
#define FRAMELEN 8
#define DEBUG 1

#define STX 0x02
#define ETX 0x03


FILE *console = NULL;

/*  initserial

  opens serial port for communication
  serial port settings:
     19200, 8, odd parity,  2stop

*/
static int 
initserial (struct termios *newtio, struct termios *oldtio, char *sdevice ) {
  int pfd;

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

  /* (v0.1.4):
     When reading from the serial port, return after 3 tenths of a second,
     even if only 0 bytes were received.  This avoids blocking the main
     loop, so that we can respond quickly to SIGHUP and SIGINT.
  */
  newtio->c_cc[VMIN]=0;       /* Was: 1 */
  newtio->c_cc[VTIME]=3;      /* Was: 5 */

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


/* v0.1.4
   Trap HUP and INTR signals, and close down cleanly.
 */
static int bTimeToQuit = 0;
void MySigHupHandler(int sig)
{
    fflush(stdout);
    fprintf(console,"SIGHUP received: ");
    bTimeToQuit = 1;
};
void MySigIntHandler(int sig)
{
    fflush(stdout);
    fprintf(console,"SIGINT received: ");
    bTimeToQuit = 1;
};


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
    int h, p, wdir, r, b, rad, mul, imul;
    int debug=0, raw=0, pipeline=0;

    struct termios newtio,oldtio; /* termios structures to set comm parameters */
    unsigned char data[MAXBUFF];
    char clk[MAXBUFF];
    char buf[MAXBUFF];
    char pipelinebuf[MAXBUFF];
    char pipelineclk[MAXBUFF];
    char *name;  /* sensor typ spec */

    time_t mtime;
    struct tm *ctm;

    console = stdout;   // Write human-readable messages to stdout by default...

    // ... but always print opening message to stderr,
    //     because we haven't read the flags yet,
    //     and pipeline-mode output flag) is not disturbed
    fprintf(stderr,"pcwsr 0.1.4 - experimental test version\n");

    while ( --argc > 0 && (*++argv)[0] == '-')
      while ( c = *++argv[0] )
	switch (c) {
	case 'p':
	  pipeline= 1;
	  console=stderr;  // switch human-readable messages from stdout to stderr
          fprintf(console,"option: pipeline\n");
	  break;
	case 'r':
	  raw = 1;
          fprintf(console,"option: raw\n");
	  break;
	case 'd':
	  debug= 1;
          fprintf(console,"option: debug\n");
	  break;
	default:
	  fprintf(console,"Illegal option: %c\n", c);
	  argc = 0;
	  break;
	}

    if (argc !=1 ) {
      fprintf(console,"Usage: pcwsr -r -d -p <serial device>\n");
      fprintf(console,"\twhere <options> include:\n");
      fprintf(console,"\t-r:\tdump raw data frames\n");
      fprintf(console,"\t-d:\taddtl. debug output\n");
      fprintf(console,"\t-p:\tpipeline-compatible output to stdout, suitable for piping to 'pcwsr_to_postgresql'\n");
      exit(-1);
    }

  
    fprintf(console,"Using serialport: %s\n", *argv);
    fflush(console);

    tzset();                                   /* setting timezone */
    fd = initserial(&newtio, &oldtio, *argv); /* serial initialization */ 
    

    
    if (signal(SIGHUP, MySigHupHandler) == SIG_ERR)  
    {
    	fprintf(console, "Can't install SIGHUP handler: exit!\n");
	exit(1);
    };
    if (signal(SIGINT, MySigIntHandler) == SIG_ERR)  
    {
    	fprintf(console, "Can't install SIGINT handler: exit!\n");
    };
     
    for ( ;; ) {
    
      // Data packet format: STX type W1 W2 W3 W4 W5 ETX   (8 bytes total).
      // Need to be careful, as STX and ETX might occur inside the data.
      
      int len=0;
      memset(data, 0, FRAMELEN);
      fflush(stdout);
      while (!bTimeToQuit  &&  len < 8)
      {
        unsigned char ch=0;
      	if (err=read(fd, &ch, 1)!=1)
		continue;	// Loop until a byte is read.
	if (len==0  &&  ch!=STX)
		continue;	// Loop until first byte is STX
	data[len++] = ch;
      };
      
      if (bTimeToQuit) 
      {
        fflush(stdout);
      	fprintf(console, "quitting..\n");
	break;
      };
 
      time(&mtime);
      ctm = gmtime(&mtime);
      strftime(clk, sizeof(clk), "%a %b %d %Y %X", ctm); // Wed Oct 08 2003 20:59:32
      strftime(pipelineclk, sizeof(pipelineclk), "'%Y-%m-%d %H:%M:%S'", ctm); 	// 1999-01-08 04:05:06

      if (raw) {
	fprintf(console,"%s : %d bytes read |", clk, err);
	for ( i = 0; i<8; i++) {
	  fprintf(console,"%x|", data[i]);
	}
	fprintf(console,"\n");
      }

      // When we get here, we should have 8 bytes starting with STX and ending with ETX.
      if (data[0]!=STX)
      {
        fflush(stdout);
      	fprintf(console, "No STX seen - can't happen!  Skipping..\n");
	continue;
      };
      if (data[7]!=ETX)
      {
        fflush(stdout);
      	fprintf(console, "No ETX seen - skipping to next STX..\n");
	continue;
      };
	      
      /* figure sensor type and address */
      dummy = data[1] & mask;
      styp  = dummy & 0xf0;
      styp  = styp >> 4;
      saddr = dummy & 0xf;
      name = sname(styp);
      strcpy(buf,"");
      strcpy(pipelinebuf,"");
     

      sver = 0x12;
      if ( ( styp < 0x5 ) && ( saddr < 0x8)) {
	  sver = 0x11;
      }

      if (debug) {
	fprintf(console,"dummy :%x, styp :%x, saddr:%x\n", dummy, styp, saddr);      
	fprintf(console,"sname: %s, styp  :%x, sver :%x\n", name, styp, sver);
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
	  fprintf(console,"Temperature\n");
	  fprintf(console,"W1: %x, W2: %x\n", data[2], data[3]);
	  fprintf(console,"hi : %x, lo: %x, Temp: %.1f\n\n", hi, lo, t);
	}
        
        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (address 0x0%x) | T[C] %.1f",
	    clk, name, saddr, t);
        }
	if (pipeline) {
	  // Example: "'2003-10-08 21:09:30',Outdoor_Temp,0x0f,T,12.3"
          snprintf(pipelinebuf, sizeof(pipelinebuf),
            "%s,Outdoor_Temp,0x0%x,T,%.1f",
	    pipelineclk, saddr, t);
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
	  fprintf(console,"Temperature\n");
	  fprintf(console,"W1: %x, W2: %x\n", data[2], data[3]);
	  fprintf(console,"hi : %x, lo: %x, Temp: %.1f\n\n", hi, lo, t);
	  fprintf(console,"Humidity\n");
	  fprintf(console,"W3: %x\n", data[4]);
	  fprintf(console,"Hum: %d\n\n", h);
	}

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | T[C] %.1f | H[%% rel.hum] %d",
	    clk, name, sver, saddr, t, h);
        }
	if (pipeline) {
	  // Example: "'2003-10-08 21:09:30',Outdoor,0x0f,T,12.3,H,42"
          snprintf(pipelinebuf, sizeof(pipelinebuf),
            "%s,Outdoor,0x0%x,T,%.1f,H,%d",
	    pipelineclk, saddr, t, h);
	}
      } 
      /* rain */
      else if ( styp == 0x2 ) {
	hi = data[2] & 0x1f ;
        hi = hi << shift;
	lo = data[3] & mask;
	r  = hi + lo ;

	if (debug) {
	  fprintf(console,"Rain\n");
	  fprintf(console,"W1: %x, W2: %x\n", data[2], data[3]);
	  fprintf(console,"hi : %x, lo: %x, Rain: %d\n\n", hi, lo, r);
	}

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | [Impulses * 0.37mm] %d",
	    clk, name, sver, saddr, r);
        }
	if (pipeline) {
	  // The impulse counter wraps around to zero eventually.
	  // Example: "'2003-10-08 21:09:30',Rain,0x0f,123"
          snprintf(pipelinebuf, sizeof(pipelinebuf),
            "%s,Rain,0x0%x,%d",
	    pipelineclk, saddr, r);
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
	  fprintf(console,"Windspeed\n");
	  fprintf(console,"W1: %x, W2: %x\n", data[2], data[3]);
	  fprintf(console,"hi : %x, lo: %x, Windspd: %.1f\n\n", hi, lo, wspd);
	  fprintf(console,"Wind Deviation\n");
	  fprintf(console,"W3: %x\n", data[4]);
	  fprintf(console,"Wind deviation: %f\n\n", wdev);
	  fprintf(console,"Wind Direction\n");
	  fprintf(console,"W4: %x, W5: %x\n", data[5], data[6]);
	  fprintf(console,"Wind direction: %d\n\n", wdir);
	}

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | Speed[km/h] %.1f | Direction [] %d +/- %.1f",
	    clk, name, sver, saddr, wspd, wdir, wdev);
        }
	if (pipeline) {
	  // Example: "'2003-10-08 21:09:30',Wind,0x0f,V,12.3,D,270,+-,22.5"
          snprintf(pipelinebuf, sizeof(pipelinebuf),
            "%s,Wind,0x0%x,V,%.1f,D,%d,+-,%.1f",
	    pipelineclk, saddr, wspd, wdir, wdev);
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
	  fprintf(console,"Temperature\n");
	  fprintf(console,"W1: %x, W2: %x\n", data[2], data[3]);
	  fprintf(console,"hi : %x, lo: %x, Temp: %.1f\n\n", hi, lo, t);
	  fprintf(console,"Humidity\n");
	  fprintf(console,"W3: %x\n", data[4]);
	  fprintf(console,"Hum: %d\n\n", h);
	  fprintf(console,"Pressure\n");
	  fprintf(console,"W4: %x, W5: %x\n", data[5], data[6]);
	  fprintf(console,"dummy: %x, hi : %x, lo: %x, Press: %d\n", dummy, hi, lo, p);
	}

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | T[C] %.1f | H[%% rel.hum] %d | P[hPa] %d",
	    clk, name, sver, saddr, t, h, p);
        }
	if (pipeline) {
	  // Example: "'2003-10-08 21:09:30',Indoor,0x0f,T,21.5,H,44,P,1009"
          snprintf(pipelinebuf, sizeof(pipelinebuf),
            "%s,Indoor,0x0%x,T,%.1f,H,%d,P,%d",
	    pipelineclk, saddr, t,h,p);
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
        // New info 2003/10: From English instructions... 
        // Units are "lux", multiplier x1, x10, x100 or x1000. Resolution 3 digits. Accuracy +- 10%.
	imul = 1;
	switch(mul)
	{
	case 0: imul=   1; break;
	case 1: imul=  10; break;
	case 2: imul= 100; break;
	case 3: imul=1000; break;
	default: ; 
	};

        if (debug) {
	  fprintf(console,"Brightness\n");
	  fprintf(console,"W1: %x, W2: %x\n", data[2], data[3]);
	  fprintf(console,"hi : %x, lo: %x, brightness: %d\n\n", hi, lo, b);
	  fprintf(console,"Multiplicator\n");
	  fprintf(console,"W3: %x\n", data[4]);
          fprintf(console,"mul: %d\n", mul);
	}

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | [klx] %d",
	    clk, name, sver, saddr, b * imul);
        }
    	if (pipeline) {
	  // Example: "'2003-10-08 21:09:30',Brightness,0x0f,123"
          snprintf(pipelinebuf, sizeof(pipelinebuf),
            "%s,Brightness,0x0%x,%d",
	    pipelineclk, saddr, b * imul);
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
	imul = 1;
	switch(mul)
	{
	case 0: imul=   1; break;
	case 1: imul=  10; break;
	case 2: imul= 100; break;
	case 3: imul=1000; break;
	default: ; 
	};

        if (debug) {
	  fprintf(console,"Pyranometer\n");
	  fprintf(console,"W1: %x, W2: %x\n", data[2], data[3]);
	  fprintf(console,"hi : %x, lo: %x, Radiation Power: %d\n\n", hi, lo, rad);
	  fprintf(console,"Multiplicator\n");
	  fprintf(console,"W3: %x\n", data[4]);
	  fprintf(console,"mul: %d\n\n", h);
	}

        if (!raw) {
          snprintf(buf, sizeof(buf),
            "%s | %8s (version %x at address 0x0%x) | Radiation Power[arb.unit] %d",
	    clk, name, sver, saddr, rad * imul);
        }
    	if (pipeline) {
	  // Example: "'2003-10-08 21:09:30',RadPower,0x0f,123"
          snprintf(pipelinebuf, sizeof(pipelinebuf),
            "%s,RadPower,0x0%x,%d",
	    pipelineclk, saddr, rad * imul);
	}
      }
      /* sensor unknown ?! */
      else {

	snprintf(buf, sizeof(buf), "%s | unknown sensor type ( possible version %x at address 0x0%x): Please report incident to: Volker.Jahns@thalreit.de",
		clk, sver, saddr);
      }
      if (!raw     && (buf[0]        !='\0')) fprintf(console,"%s\n", buf);
      if (pipeline && (pipelinebuf[0]!='\0')) fprintf(stdout, "%s\n", pipelinebuf);
      fflush(stdout);
    }
    /* leave serial line in good state */
    closeserial(fd, &oldtio); 
    return(0);
}









