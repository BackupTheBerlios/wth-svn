/* 

   onewire.c

   wth communication w/ onewire sensors

   $Id$
   $Revision$

   Copyright (C) 2008 Volker Jahns <volker@thalreit.de>

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
#include "ds2438.h"

/*
 *   ppagemem
 *
 *     print contents of uchar array of memory
 *
 *     */
char 
*ppagemem( uchar *pagemem) {
  int x;
  char tchr[4];
  static char pblock[NBUFF + 1] = "";

  sprintf( pblock, "");
  for ( x = 0; x < 10; x++) {
    snprintf( tchr, 4, "%x:", pagemem[x]);
    strncat( pblock, tchr, 4);
  }
  return pblock;
}

/*
  bitprint 

  utility function to print bits of 8-bit byte
*/
int 
bitprint ( int byte, char *s_reg ) {
  int x;
  struct timezone tz;
  struct timeval  tv;

  gettimeofday(&tv, &tz);
  syslog(LOG_INFO, "%5s | %lu.%lu : #", s_reg, tv.tv_sec, tv.tv_usec);
  for( x = 7; x > -1; x-- )
    syslog(LOG_INFO, "%i", (byte & 1 << x ) > 0 ? 1 : 0 );
  return(0);
}

/*
 *   longprint 
 *
 *     utility function to print bits of 16-bit byte
 *     */
int 
longprint ( int byte, char *s_reg ) {
  int x;
  struct timezone tz;
  struct timeval  tv;

  gettimeofday(&tv, &tz);
  syslog(LOG_INFO, "%5s | %lu.%lu : #", s_reg, tv.tv_sec, tv.tv_usec);
  for( x = 15; x > -1; x-- )
    syslog(LOG_INFO, "%i", (byte & 1 << x ) > 0 ? 1 : 0 );
  return(0);
}
/*
   echo_serialnum

   returns serial number of 1-Wire device in char array

*/
char *echo_serialnum( uchar serialnum[]) {
  int i;
  static char rbuf[64];
  char buf[64];

  snprintf(rbuf, 64, "");
  for ( i = 7; i >= 0; i--) {
    snprintf(buf, 3, "%02X", (int)serialnum[i]);
    strcat(rbuf, buf);
  }
  return(rbuf);
}

/*
 
  echo_familycode

  returns familycode of 1-Wire device

*/
char 
echo_familycode( uchar serialnum[]) {
  return((char)serialnum[0]);
}


/*
   onewire_handler
   logging onewire sensor data to rrd and Sqlite DB
*/
void *
onewire_hd( void *arg) {
  int i, nmeas, rslt;
  int portnum;
  float vsens;
  double mtime;
  uchar serialnum[9];
  char port[256] = "";
  struct timezone tz;
  struct timeval  tv;

  syslog( LOG_DEBUG, "onewire_hd: start of execution\n");

  if((portnum = owAcquireEx(port)) < 0) {
    OWERROR_DUMP(stdout);
    exit(1);
  }

  for (;;) {
    /* measurement loop */
    for ( i = 0; i < nmeas; i++) {
      /* find the first device (all devices not just alarming) */
      rslt = owFirst( portnum, TRUE, FALSE);
      while (rslt) {
        /* print the Serial Number of the device just found */
        owSerialNum( portnum,serialnum,TRUE);
        if ( echo_familycode(serialnum) == 0x26 ) {
          gettimeofday( &tv, &tz);
          mtime = tv.tv_sec+1.0e-6*tv.tv_usec; 
          vsens = ReadVsens( 0, VSENS, serialnum, port);
          syslog(LOG_DEBUG, "%f\t%s\t%f\n", 
		 mtime, echo_serialnum( serialnum), vsens);
          /*
          if ( verbose == 1 ) {
            ds2438mem_dump(0, TRUE, serialnum, port);
          }
          */
        }
        /* find the next device */
        rslt = owNext( portnum, TRUE, FALSE);
      } 
    }
  }

  /* release the 1-Wire Net */
  owRelease( portnum);
  syslog( LOG_INFO, "Closing port: %s\n", port);
  return( ( void *) &success);
}
