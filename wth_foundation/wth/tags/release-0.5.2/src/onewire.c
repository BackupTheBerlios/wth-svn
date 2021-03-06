/* 

   onewire.c

   wth communication w/ onewire sensors

   $Id$
   $Revision$

   Copyright (C) 2008-2010 Volker Jahns <volker@thalreit.de>

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
#include <sys/resource.h>

struct mset {
  double mtime;
  float mval;
  struct mset *next;
};

/*
  ppagemem

  print contents of uchar array of memory
*/
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

  adding measurement data in linked list mlist
*/
int addmdat( struct mset ** mlist_ref, double mtime, double mval) {
  struct mset *meas_set;
  if ( ( meas_set = ( struct mset *)malloc ( sizeof ( struct mset))) == NULL ) {
    return 1;
  } else {
    meas_set->mtime = mtime;
    meas_set->mval  = mval;
    meas_set->next  = *mlist_ref;
    *mlist_ref = meas_set;
    return 0;
  }
}

/*
  rstmdat

  delete linked list mlist_p and set the head pointer to NULL
*/
void rstmdat( struct mset ** mlist_ref) {
  struct mset * current = *mlist_ref;
  struct mset * next;

  while ( current != NULL ) {
    next = current->next;
    free(current);
    current = next;
  }  
  *mlist_ref = NULL;
}

/*
  prtmdat

  print data in linked list
*/
void prtmdat( struct mset *mlist_p) {

  if ( mlist_p != NULL) {
    prtmdat(mlist_p->next);
    syslog(LOG_DEBUG, "prtmdat: meas_set->mtime: %f, meas_set->mval: %f\n", 
	   mlist_p->mtime, mlist_p->mval);
  }
}

/*
  avgmdat

  average data in linked list and write to database
*/
void avgmdat( struct mset ** mlist_ref, int sens_meas_no) {
  struct mset *llist_p = *mlist_ref;
  int count = 0;
  double avgtime = 0;
  double avgval  = 0;

  while ( llist_p != NULL) {
    count++;
    avgtime = avgtime + llist_p->mtime;
    avgval = avgval + llist_p->mval;
    llist_p = llist_p->next;
  }
  if ( count != 0 ) {
    avgtime = avgtime / count;
    avgval = avgval / count;
    syslog( LOG_DEBUG, 
	    "avgmdat: sens_meas_no: %d, avgtime: %f, avgval: %f, number: %d", 
	    sens_meas_no, avgtime, avgval, count); 
    datadb( avgtime, sens_meas_no, avgval, onewiredb);
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
  float vsens, vad, vdd, temp10;
  float press2438, humid2438;
  float temp;
  double mtime, temp2438;
  uchar serialnum[9];
  char port[TBUFF+1];
  char *errmsg;
  sensdevpar_t ssdp;
  struct timezone tz;
  struct timeval  tv;
  struct mset *mlist_p[MAXSENSMEAS];
  /*
  time_t pstatime;
  struct tm *pstatm;
  struct rusage pstat;
  */

  syslog( LOG_DEBUG, "onewire_hd: start of execution");

  // initialize parameters
  syslog(LOG_DEBUG,"onewire_hd: onewirestation.config.device: %s", 
	 onewirestation.config.device);
  strncpy( port,  onewirestation.config.device, TBUFF);
  if((portnum = owAcquireEx(port)) < 0) {
    onewirestation.status.is_present = -1;
    OWERROR_DUMP(stdout);
    return( ( void *) &failure);
  }
  onewirestation.status.is_present = 1;
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
  syslog(LOG_DEBUG, "onewire_hd: max_sens_meas: %d", max_sens_meas);

  for (;;) {
    /* measurement loop */
    syslog( LOG_DEBUG, "onewire_hd: start of measurement loop");
    for ( i = 0; i < onewirestation.config.mcycle; i++) {
      gettimeofday( &tv, &tz);
      mtime = tv.tv_sec+1.0e-6*tv.tv_usec; 
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

          /* read DS2438 VSENS */
          vsens = ReadVsens( 0, VSENS, serialnum, port);
          if ( ( rslt  = sensdevpar( "VSENS+", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS2438(VSENS+): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            syslog(LOG_DEBUG, "onewire_hd: call to addmdat w/ mtime: %f and mval: %f\n", mtime, vsens);
            addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, vsens);
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem relation sensorname, parametername devicetyp: check database setup of serialnum: %s", echo_serialnum(serialnum));
          }

          /* read DS2438 VAD */
          svdd = 0;
          SetupAtoD( portnum, svdd, serialnum);
          vad = ReadAtoD( portnum, svdd, serialnum);
          if ( ( rslt  = sensdevpar( "VAD", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS2438(VAD): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, vad);
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem relation sensorname, parametername devicetyp: check database setup of serialnum: %s", echo_serialnum(serialnum));
          }

          /* read DS2438 VDD */
          svdd = 1;
          SetupAtoD( portnum, svdd, serialnum);
          vdd = ReadAtoD( portnum, svdd, serialnum);

          if ( ( rslt = sensdevpar( "VDD", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS2438(VDD): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, vdd);
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem relation sensorname, parametername devicetyp: check database setup of serialnum: %s", echo_serialnum(serialnum));
          }

          /* read DS2438 temperature */
          temp2438 = Get_Temperature( portnum, serialnum);
          if ( ( rslt = sensdevpar( "Temperature", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: ds2438(Temperature): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, temp2438);
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem relation sensorname, parametername devicetyp: check database setup of serialnum: %s", echo_serialnum(serialnum));
          }

          /* humidity - derived quantity calculated from vad and vdd */
          if ( ( rslt = sensdevpar( "Humidity", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG,
              "onewire_hd: DS2438(Humidity): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            cvad = vad;
            cvdd = vdd;
            if ( ( cvad > 0) || ( cvdd > 0)) {
              humid2438 = ( ( vad/vdd) - 0.16) * 161.29;
              humid2438 = humid2438 / ( 1.0546 - ( 0.00216 * temp2438));
	      addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, humid2438);
              syslog(LOG_DEBUG,"Humidity sensor discovered: %f", humid2438);
            } else {
              syslog(LOG_ALERT,"Humidity : negative value of VAD or VDD: skipping!");
            }
          }

          /* pressure - derived quantity calculated from vad and vdd */
          if ( ( rslt = sensdevpar( "Pressure", echo_serialnum( serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG,
              "onewire_hd: DS2438(Pressure): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            cvad = vad;
            cvdd = vdd;
            if ( ( cvad > 0) || ( cvdd > 0)) {
              press2438 = 110.76 * ( vad/vdd) + 850.34;
	      addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, press2438);
              syslog(LOG_DEBUG,"Pressure sensor discovered: %f", press2438);
            } else {
              syslog(LOG_ALERT,"Pressure: negative value of VAD or VDD: skipping!");
            }
          }

          syslog(LOG_DEBUG, 
            "%f DS2438 serialnum: %s VSENS: %f VAD: %f VDD: %f Temperature: %f\n", 
	    mtime, echo_serialnum( serialnum), vsens, vad, vdd, temp2438);
          /*
          if ( verbose == 1 ) {
            ds2438mem_dump(0, TRUE, serialnum, port);
          }
          */
        /* DS1820/DS1920 */
        } else if ( strncmp(echo_familycode(serialnum), "10",1) == 0 ) {
          if ( ( rslt = ReadTemperature( portnum, serialnum, &temp)) == 1 ) {
            temp10 = temp;
            syslog(LOG_DEBUG, "%f DS1820/DS1920 serialnum: %s Temperature: %f",
		   mtime, echo_serialnum(serialnum),temp10); 
          } else {
            syslog(LOG_ALERT, 
	      "%f DS1820/DS1920 serialnum: %s Temperature conversion error", 
	      mtime, echo_serialnum( serialnum)); 
          }

          if ( ( rslt = sensdevpar( "Temperature", echo_serialnum(serialnum), &ssdp, onewiredb)) == 0 ) {
            syslog(LOG_DEBUG, 
              "onewire_hd: DS1820/DS1920 (Temperature): ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, temp10);
          }
        }

        /* find the next device */
        rslt = owNext( portnum, TRUE, FALSE);
      } 
      syslog(LOG_DEBUG, 
        "onewire_hd: number of 1-wire devices detected: %d", numsens);
      if ( currsens != numsens) {
        syslog(LOG_ALERT, 
          "Number of 1-Wire devices changed from %d to %d", 
          currsens, numsens);  
      }
      currsens = numsens;
      syslog(LOG_DEBUG,"1-wire loop: mcycle: %d %f %f %f %f %f %f %f", 
        i, mtime, vsens, vad, vdd, temp2438, humid2438, temp10);
    }
    syslog( LOG_DEBUG, "onewire_hd: end of measurement loop");
    syslog( LOG_DEBUG, "onewire_hd: start of averaging");
    for ( j = 0 ; j < MAXSENSMEAS; j ++ ) {
      avgmdat( &mlist_p[j], j);
      rstmdat( &mlist_p[j]);
    }
    syslog( LOG_DEBUG, "onewire_hd: end of averaging");
  }

  /* database cleanup and close */
  sqlite3_close( onewiredb);

  /* release the 1-Wire Net */
  owRelease( portnum);
  syslog( LOG_INFO, "Closing port: %s\n", port);
  return( ( void *) &success);
}
