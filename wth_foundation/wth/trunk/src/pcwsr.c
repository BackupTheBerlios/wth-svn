/* pcwsr.c

   serial line communication w/ PC weathersensor receiver

   $Id$
   $Revision$

   Copyright (C) 2001-2004,2007,2008 Volker Jahns <volker@thalreit.de>

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

/*  initpcwsr

  opens serial port for communication with PC weathersensor receiver
  serial port settings:
  PCWSR (PC Weatherstation receiver by ELV)
     19200 BAUD, 8, Odd Parity, 2 Stopp


*/
static int 
initpcwsr (struct termios *newtio, struct termios *oldtio, char *sdevice ) {
  int pfd;

  if ( ( pfd = open( sdevice, O_RDWR| O_NOCTTY| O_NDELAY| O_NONBLOCK)) 
           == -1 ) {
    perror("initpcwsr: Unable to open serial device");
  } 
  
  /* Make the file descriptor asynchronous (the manual page says only 
     O_APPEND and O_NONBLOCK, will work with F_SETFL...) */
  fcntl(pfd, F_SETFL, FASYNC); 
  
  /* save current port settings */
  tcgetattr(pfd,oldtio);
  
  /* prefill port settings */
  tcgetattr(pfd,newtio);

  /* set communication link parameters */
  cfsetispeed(newtio, B19200);  
  cfsetospeed(newtio, B19200);

  
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
   closepcwsr

   restore the old port settings
   close port

*/
static int closepcwsr( int fd, struct termios *oldtio) {

    /* restore old port settings */
    tcsetattr(fd,TCSANOW,oldtio);
	
    /* close serial port */
    close(fd);
    return(0);
}


/*
   pcwsr_loghandler
   logging pcwsr data to rrd and Sqlite DB
*/
void *
pcwsr_hd( void *arg) {
    int i, fd, err;
    int hi, lo, dummy;
    int mask = 0x7f;
    int shift = 0x7;
    int len, nval;
    int sensor_no, sensor_meas_no[3];
    int styp, saddr, sver;
    float t, wspd, wdev;
    int hum, pres, wdir, r, b, rad, mul, imul;
    float meas_value[3];
   
    struct termios newtio,oldtio; 
    unsigned char data[MAXMSGLEN+1];
    char clk[MAXMSGLEN+1];
    char msg[TBUFF+1];
    char tstrg[TBUFF+1];
    char rrdfile[TBUFF+1];
    char template[TBUFF+1];
    char **ustrg;
    static char buf[TBUFF+1];
    time_t dataset_date;
    struct tm *ctm;
    senspar_t spar;

    syslog( LOG_DEBUG, "pcwsr_hd: start of execution\n");

    if (( ustrg = malloc(sizeof(char *)*MAXMSGLEN+1)) == NULL )
      return( ( void *) &failure);

    if (( ustrg[2] = malloc(sizeof(char)*NBUFF+1)) == NULL)
      return( ( void *) &failure);

    /* setting timezone */
    tzset(); 
    /* serial initialization */ 
    fd = initpcwsr(&newtio, &oldtio, pcwsrstation.config.device); 

    if ( ( err = sqlite3_open( pcwsrstation.config.dbfile, &pcwsrdb))) {
      snprintf(msg, TBUFF, "Failed to open database %s. Error: %s\n",
	       pcwsrstation.config.dbfile, sqlite3_errmsg( pcwsrdb));
      closepcwsr( fd, &oldtio); 
      return( ( void *) &failure);
    }
    
    /* main loop to read data and write db */
    for ( ;; ) {
      buf[0] = '\0';
      len=0;
      nval = 0;
      memset(data, 0, PCWSRLEN);
      // Data packet format: STX type W1 W2 W3 W4 W5 ETX   (8 bytes total).
      // Need to be careful, as STX and ETX might occur inside the data.
      while ( len < 8)
      {
        unsigned char ch=0;
      	if ( ( err=read(fd, &ch, 1)) !=1)
		continue;	// Loop until a byte is read.
	if ( len==0  &&  ch!=STX)
		continue;	// Loop until first byte is STX
	data[len++] = ch;
      };
      time(&dataset_date);
      ctm = gmtime(&dataset_date);
      strftime(clk, sizeof(clk), "%a %b %d %Y %X", ctm); 

     /* When we get here, we should have 8 bytes 
         starting with STX and ending with ETX. */
      if (data[0]!=STX)
      {
      	printf("pcwsr_hd: no STX seen - can't happen!  Skipping..\n");
	continue;
      };
      if (data[7]!=ETX)
      {
      	printf("pcwsr_hd: no ETX seen - skipping to next STX..\n");
	continue;
      };
	      
      /* figure sensor type and address */
      dummy = data[1] & mask;
      styp  = dummy & 0xf0;
      styp  = styp >> 4;
      saddr = dummy & 0xf;
      sensor_no = dummy;
      syslog( LOG_DEBUG, "pcwsr_hd: styp: %d : saddr: %d : sensor_no: %d \n", 
        styp, saddr, sensor_no);
      strcpy(buf,"");
     
      sver = 0x12;
      if ( ( styp < 0x5 ) && ( saddr < 0x8)) {
	  sver = 0x11;
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
        sensor_meas_no[0] = saddr + 1;
        nval++;
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
        sensor_meas_no[0] = 17 + 2*saddr + 0; 
        meas_value[0]=t;
        nval++;
	/* humidity */
	hum = data[4] & mask;
        sensor_meas_no[1] = 17 + 2*saddr + 1;
        meas_value[1] = hum;
        nval++;
      } 
      /* rain */
      else if ( styp == 0x2 ) {
	hi = data[2] & 0x1f ;
        hi = hi << shift;
	lo = data[3] & mask;
	r  = hi + lo ;
        sensor_meas_no[0] = 49 + saddr;
        meas_value[0] = r;
        nval++;
      }
      /* wind */
      else if ( styp == 0x3 ) {
	/* wind speed*/
	hi = data[2] & mask ;
        hi = hi << shift;
	lo = data[3] & mask;
	wspd  = ( hi + lo ) / 10.0 ;
        sensor_meas_no[0] = 65 + 3*saddr + 0; 
        meas_value[0] = wspd;
        nval++;

	/* wind deviation */
	dummy = data[4] & mask;
        wdev  = (dummy * 45 );
        wdev  = 0.5 * wdev;
        sensor_meas_no[1] = 65 + 3*saddr + 1; 
        meas_value[1] = wdev;
        nval++;

	/* wind direction */
	hi = data[5] & mask ;
        hi = hi << shift;
	lo = data[6] & mask;
	wdir = hi + lo ;
        sensor_meas_no[2] = 65 + 3*saddr + 2;
        meas_value[2] = wdev;
        nval++;
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
        sensor_meas_no[0] = 113 + 3*saddr + 0; 
        meas_value[0] = t;
        nval++;

	/* humidity */
	hum = data[4] & mask;
        sensor_meas_no[1] = 113 + 3*saddr + 1; 
        meas_value[1] = hum;
        nval++;

	/* pressure */
	dummy  = data[5] & mask;
	hi = dummy << shift;
	lo = data[6] & mask;
	pres  = hi + lo;
        sensor_meas_no[2] = 113 + 3*saddr + 2; 
        meas_value[2] = pres;
        nval++;
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
        // Units are "lux", multiplier x1, x10, x100 or x1000. 
        // Resolution 3 digits. Accuracy +- 10%.
	imul = 1;
	switch(mul)
	{
	case 0: imul=   1; break;
	case 1: imul=  10; break;
	case 2: imul= 100; break;
	case 3: imul=1000; break;
	default: ; 
	};
        meas_value[0] = b * imul;
        sensor_meas_no[0] = 161 + saddr ;
        nval++;
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
        sensor_meas_no[0] = 170 + saddr;
        meas_value[0] = rad * imul;
      }
      /* sensor unknown ?! */
      else {
	snprintf(buf, sizeof(buf), 
	  "%s | unknown sensor type "
          "( possible version %x at address 0x0%x): "
	  "Please report incident to: Volker.Jahns@thalreit.de",
	  clk, sver, saddr);
      }

      /* set PCWSR present flag */
      pcwsrstation.status.is_present = 1;
      printf( "pcwsrstation.status.is_present: %d\n",
        pcwsrstation.status.is_present); 

      /* database and rrd handling */
      snprintf(tstrg,MAXMSGLEN, "%d", dataset_date);
      for ( i = 0; i < nval; i++) 
      {
        datadb( dataset_date, sensor_meas_no[i], meas_value[i], pcwsrdb);
        /* handling rrd */
        /* fetch names from database */
        if ( ( err = senspardb( sensor_meas_no[i], &spar, pcwsrdb)) != 0 ) {
	  syslog(LOG_DEBUG,"pcwsr_hd: senspardb returned error: %d\n", err);
          break;
	} else {
          if ( spar.sensor_no != sensor_no ) {
	    syslog(LOG_WARNING, 
              "pcwsr_hd: sensor_no mismatch: sensor_no: %d : "
              "spar.sensor_no: %d\n", sensor_no, spar.sensor_no);
            break;
	  }
          syslog(LOG_DEBUG, 
            "pcwsr_hd: sensor_meas_no: %d : spar.sensor_no: %d: "
            "spar.sensor_name: %s: spar.par_name: %s\n", 
	    sensor_meas_no[i], spar.sensor_no, spar.sensor_name, spar.par_name);
          syslog(LOG_INFO, 
            "pcwsr_hd: %lu : sensor: %s%d : parameter: %s: %f\n",
	    (long int)dataset_date, spar.sensor_name, spar.sensor_no, 
            spar.par_name, meas_value[i]);
          snprintf(template,MAXMSGLEN,"%f", meas_value[i]);
          strncat(tstrg, ":", 1);
          strncat(tstrg, template, strlen(template));
        }
      }
      if ( err == 0 ) {
        snprintf( rrdfile, MAXMSGLEN, "%s%s%d.rrd", 
          pcwsrstation.config.rrdpath, spar.sensor_name, spar.sensor_no);
        syslog(LOG_DEBUG, "pcwsr_hd: rrdfile: %s: update string: %s\n", 
          rrdfile, tstrg);
        snprintf(ustrg[2], MAXMSGLEN-2, "%s", tstrg);
        rrd_clear_error();
        rrd_get_context();
        rrd_update_r( rrdfile, NULL, 1, (const char **)(ustrg + 2));
        if ( rrd_test_error()) {
          syslog( LOG_ALERT, "pcwsr_hd: RRD Error: %s\n", rrd_get_error());
        }
      }
    }

    /*cleanup and close database */
    sqlite3_close( pcwsrdb);
    /* leave serial line in good state */
    closepcwsr( fd, &oldtio); 
    return( ( void *) &success);
}
