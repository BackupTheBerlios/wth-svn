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
#include "temp10.h"

typedef struct mset mset_t;
struct mset {
  double mtime;
  float mval;
  mset_t *next;
};


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
  syslog(LOG_INFO, "%5s | %lu.%lu : #", s_reg, (long unsigned int) tv.tv_sec, (long unsigned int)tv.tv_usec);
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
  syslog(LOG_INFO, "%5s | %lu.%lu : #", s_reg, (long unsigned int) tv.tv_sec, (long unsigned int) tv.tv_usec);
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
char *echo_familycode( uchar serialnum[]) {
  static char rbuf[TBUFF];
  snprintf( rbuf, TBUFF-1, "%x", serialnum[0]);
  return(rbuf);
}

/*
  addmdat

  adding measurement data in structure mset

*/
mset_t * addmdat( mset_t *listp, mset_t *newp) {
  newp->next = listp;
  return newp;
}

/* 
  newmdat

  create new mset for mtime and mval
*/
mset_t
*newmdat( double mtime, float mval) {
  mset_t *newp;
  newp = ( mset_t *) malloc(sizeof(mset_t));
  newp->mtime = mtime;
  newp->mval = mval;
  newp->next = NULL;
  return newp;
}


/*
  pmdat

*/
void pmdat( mset_t *listp) {
  int i;
  for ( ; listp != NULL; listp = listp-> next) {
    syslog(LOG_DEBUG, "pmdat: %d %f, %f\n", i, listp->mtime, listp->mval);
    i++;
  }

}

/*
   onewire_handler
   logging onewire sensor data to rrd and Sqlite DB
*/
void *
onewire_hd( void *arg) {
  int i, j, rslt, verbose, currsens, numsens;
  int portnum, svdd, cvdd, cvad;
  int max_sens_meas;
  float meas_value[TBUFF+1][TBUFF+1];
  float vsens[TBUFF+1], vad[TBUFF+1], vdd[TBUFF+1], temp10[TBUFF+1], humid2438[TBUFF+1];
  float temp;
  double mtime[TBUFF+1], temp2438[TBUFF+1];
  uchar serialnum[9];
  char port[TBUFF+1];
  char *errmsg;
  sensdevpar_t ssdp;
  struct timezone tz;
  struct timeval  tv;
  mset_t meas_set[MAXSENSORS];

  syslog( LOG_DEBUG, "onewire_hd: start of execution");

  // initialize parameters
  syslog(LOG_DEBUG,"onewire_hd: onewirestation.config.device: %s", onewirestation.config.device);
  strncpy( port,  onewirestation.config.device, TBUFF);
  if((portnum = owAcquireEx(port)) < 0) {
    OWERROR_DUMP(stdout);
    exit(1);
  }
  rslt = sqlite3_open( onewirestation.config.dbfile, &onewiredb);
  syslog(LOG_DEBUG,
    "readdb: sqlite3_open %s return value: %d : sqlite_errmsg: %s\n",
    onewirestation.config.dbfile,
    rslt, sqlite3_errmsg(onewiredb));
  if ( rslt) {
    syslog( LOG_ALERT, "readdb: failed to open database %s. error: %s\n",
    onewirestation.config.dbfile, sqlite3_errmsg(onewiredb));
    free( errmsg);
    return( ( void *) &failure);
  } else {
    syslog(LOG_DEBUG, "readdb: sqlite3_open: OK\n");
  }
    
  syslog(LOG_DEBUG, "onewire_hd: mcycle: %d", onewirestation.config.mcycle);
  verbose = 0;

  max_sens_meas = maxsensmeas( onewiredb);

  for (;;) {
    /* measurement loop */
    for ( i = 0; i < onewirestation.config.mcycle; i++) {
      gettimeofday( &tv, &tz);
      mtime[i] = tv.tv_sec+1.0e-6*tv.tv_usec; 
      /* find the first device (all devices not just alarming) */
      rslt = owFirst( portnum, TRUE, FALSE);
      numsens = 0;
      while (rslt) {
        /* print the Serial Number of the device just found */
        owSerialNum( portnum,serialnum,TRUE);
        numsens++;
        syslog( LOG_DEBUG, "1-wire device: serialnum: %s: familycode: %s", 
          echo_serialnum(serialnum), echo_familycode( serialnum));

        /* DS2438 */
        if ( strncmp(echo_familycode(serialnum), "26",1) == 0 ) {
          /* read VSENS */
          vsens[i] = ReadVsens( 0, VSENS, serialnum, port);
          if ( ( rslt  = sensdevpar( "VSENS+", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS2438(VSENS+): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            meas_value[ssdp.sensor_meas_no][i] = vsens[i];
            addmdat( meas_set, newmdat( mtime[i], vsens[i]));
            
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem relation sensorname, parametername devicetyp: check database setup");
          }

          /* read VAD */
          svdd = 0;
          SetupAtoD( portnum, svdd, serialnum);
          vad[i] = ReadAtoD( portnum, svdd, serialnum);
          if ( ( rslt  = sensdevpar( "VAD", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS2438(VAD): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            meas_value[ssdp.sensor_meas_no][i] = vad[i];
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem relation sensorname, parametername devicetyp: check database setup");
          }

          /* read VDD */
          svdd = 1;
          SetupAtoD( portnum, svdd, serialnum);
          vdd[i] = ReadAtoD( portnum, svdd, serialnum);
          if ( ( rslt = sensdevpar( "VDD", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS2438(VDD): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            meas_value[ssdp.sensor_meas_no][i] = vdd[i];
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem relation sensorname, parametername devicetyp: check database setup");
          }

          /* read temperature */
          temp2438[i] = Get_Temperature( portnum, serialnum);
          if ( ( rslt = sensdevpar( "Temperature", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: ds2438(Temperature): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            meas_value[ssdp.sensor_meas_no][i] = temp2438[i];
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem relation sensorname, parametername devicetyp: check database setup");
          }

          if ( ( rslt = sensdevpar( "Humidity", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS2438(Humidity): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            cvad = vad[i];
            cvdd = vdd[i];
            if ( ( cvad > 0) || ( cvdd > 0)) {
                humid2438[i] = ( ( vad[i]/vdd[i]) - 0.16) * 161.29;
                humid2438[i] = humid2438[i] / ( 1.0546 - ( 0.00216 * temp2438[i]));
                meas_value[ssdp.sensor_meas_no][i] = humid2438[i];
                syslog(LOG_DEBUG,"Humidity sensor discovered: %f", humid2438[i]);
            } else {
                syslog(LOG_ALERT,"Humidity : negative value of VAD or VDD: skipping!");
            }
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem relation sensorname, parametername devicetyp: check database setup");
          }
          syslog(LOG_DEBUG, "%f DS2438 serialnum: %s VSENS: %f VAD: %f VDD: %f Temperature: %f\n", 
		 mtime[i], echo_serialnum( serialnum), vsens[i], vad[i], vdd[i], temp2438[i]);
          /*
          if ( verbose == 1 ) {
            ds2438mem_dump(0, TRUE, serialnum, port);
          }
          */
        /* DS1820/DS1920 */
        } else if ( strncmp(echo_familycode(serialnum), "10",1) == 0 ) {
          if ( ( rslt = ReadTemperature( portnum, serialnum, &temp)) == 1 ) {
            temp10[i] = temp;
            syslog(LOG_DEBUG, "%f DS1820/DS1920 serialnum: %s Temperature: %f", mtime[i], echo_serialnum(serialnum),temp10[i]); 
          } else {
            syslog(LOG_ALERT, "%f DS1820/DS1920 serialnum: %s Temperature conversion error", mtime[i], echo_serialnum( serialnum)); 
          }
          if ( ( rslt = sensdevpar( "Temperature", echo_serialnum(serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS1820/DS1920 (Temperature): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 

          }
        }
        /* find the next device */
        rslt = owNext( portnum, TRUE, FALSE);
    
      } 
      syslog(LOG_DEBUG, "onewire_hd: number of 1-wire devices detected: %d", numsens);
      if ( currsens != numsens) {
        syslog(LOG_ALERT, "Number of 1-Wire devices changed from %d to %d", currsens, numsens);  
      }
      currsens = numsens;
      syslog(LOG_DEBUG,"1-wire loop: mcycle: %d %f %f %f %f %f %f %f", i, mtime[i], vsens[i], vad[i], vdd[i], temp2438[i], humid2438[i], temp10[i]);
    }
    syslog( LOG_DEBUG, "1-wire collect loop done. Averaging");
    syslog( LOG_DEBUG, "onewire_hd: onewirestation.config.mcycle: %d, max_sens_meas: %d", onewirestation.config.mcycle, max_sens_meas);
    for ( i = 0; i < onewirestation.config.mcycle; i++) {
      for ( j = 1; j <= max_sens_meas; j++) {
        syslog( LOG_DEBUG, "mcycle: %d mtime:%f meas_value[%d][%d]: %f", i, mtime[i], j, i, meas_value[j][i]);
        datadb(mtime[i], j, meas_value[j][i], onewiredb);
      }
    }
    pmdat(meas_set);
  }

  /* database cleanup and close */
  sqlite3_close( onewiredb);

  /* release the 1-Wire Net */
  owRelease( portnum);
  syslog( LOG_INFO, "Closing port: %s\n", port);
  return( ( void *) &success);
}
