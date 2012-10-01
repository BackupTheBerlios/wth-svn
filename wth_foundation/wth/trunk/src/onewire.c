/* 

   onewire.c

   wth communication w/ onewire sensors

   $Id$
   $Revision$

   Copyright (C) 2008-2012 Volker Jahns <volker@thalreit.de>

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
 
#include <wth.h>
#include <ds2438.h>
#include <temp10.h>
#include <sys/resource.h>


/*
  ppagemem

  print contents of uchar array of memory
*/
char 
*ppagemem( uchar *pagemem) 
{
  int x;
  char tchr[4];
  static char pblock[NBUFF + 1] = "";

  memset( pblock, 0, NBUFF);
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
char 
*echo_serialnum( uchar serialnum[]) 
{
  int i;
  static char rbuf[65];
  char buf[65];

  memset( rbuf, 0, 64);
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
*echo_familycode( uchar serialnum[]) 
{
  static char rbuf[TBUFF];
  snprintf( rbuf, TBUFF-1, "%x", serialnum[0]);
  return(rbuf);
}

/*
  addmdat

  adding measurement data in linked list mlist
*/
int 
addmdat( struct mset ** mlist_ref, 
         double mtime, 
         double mval) 
{
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
void 
rstmdat( struct mset ** mlist_ref) 
{
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
void 
prtmdat( struct mset *mlist_p) 
{
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
void 
avgmdat( struct mset ** mlist_ref, 
         int sens_meas_no) 
{
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
    if ( isdefined_sqlite("onewirestation") == TRUE ) {
      datadb( avgtime, sens_meas_no, avgval, onewiredb);
    } else if ( isdefined_pgsql("onewirestation") == TRUE ) {
      pg_datadb( avgtime, sens_meas_no, avgval, pg_conn);
    }
  }
}


int
maxsensmeas( char * dbtype) 
{
  int p_maxsensmeas = -1;

  if ( strncmp( dbtype, "sqlite", 6) == 0 ) {
    p_maxsensmeas = sqlite_maxsensmeas( onewiredb);
  } else if ( strncmp( dbtype, "postgresql", 10) == 0) {
    p_maxsensmeas = pg_maxsensmeas( pg_conn);
  }
  return(p_maxsensmeas);
}

int
get_onewireinfo( char *parname, 
                 char *serialnum, 
                 sensdevpar_t *ssdp, 
                 char *dbtype) 
{
  int res;

  if ( strncmp( dbtype, "sqlite", 6) == 0 ) {
    res  = sqlite_get_onewireinfo( parname,  serialnum, ssdp, onewiredb);
  } else if ( strncmp( dbtype, "postgresql", 10) == 0) {
    res  = pg_get_onewireinfo( parname,  serialnum, ssdp, pg_conn);

  }
  return(res);
}

/*
   onewire_handler

   logging onewire sensor data to sqlite database
*/
void *
onewire_hd( void *arg) 
{
  int i, j, err, rslt, verbose, currsens, numsens;
  int portnum, svdd, cvdd, cvad;
  int max_sens_meas;
  float vsens, vad, vdd, temp10;
  float press2438, humid2438;
  float temp;
  double mtime, temp2438;
  uchar serialnum[9];
  char port[TBUFF+1];
  sensdevpar_t ssdp;
  struct timezone tz;
  struct timeval  tv;
  struct mset *mlist_p[MAXSENSMEAS];

  syslog( LOG_DEBUG, "onewire_hd: start of execution");

  // initialize parameters
  syslog(LOG_DEBUG,"onewire_hd: onewirestation.config.device: %s", 
	 onewirestation.config.device);
  strncpy( port,  onewirestation.config.device, TBUFF);
  if((portnum = owAcquireEx(port)) < 0) {
    onewirestation.status.is_present = -1;
    OWERROR_DUMP(stdout);
    syslog(LOG_ALERT,"onewire_hd: owAcquireEx error");
    return( ( void *) &failure);
  }
  onewirestation.status.is_present = 1;
  currsens = 0;

  if ( isdefined_sqlite("onewirestation") == TRUE ) {
    if ( ( err = sqlite3_open( onewirestation.config.dbfile, &onewiredb))) {
      syslog(LOG_ALERT, "onewire_hd: Failed to open database %s.",
        onewirestation.config.dbfile);
      return( ( void *) &failure);
    }
  } else if ( isdefined_pgsql("onewirestation") == TRUE ) {
    pg_conn = PQconnectdb( onewirestation.config.dbconn);
    if (PQstatus(pg_conn) != CONNECTION_OK)
    {
        syslog(LOG_ALERT, "onewire_hd: connection to database failed: %s",
                PQerrorMessage(pg_conn));
        PQfinish(pg_conn);
        return( ( void *) &failure);
    }
  } else {
    syslog(LOG_ALERT, "onewire_hd: no database type defined");
    return( ( void *) &failure);
  }

  syslog(LOG_DEBUG, "onewire_hd: mcycle: %d", onewirestation.config.mcycle);
  verbose = 0;

  max_sens_meas = maxsensmeas(onewirestation.config.dbtype);

  if ( max_sens_meas <= 0 ) {
    syslog(LOG_ALERT, "onewire_hd: database configuration error : "
      "verify table SENSORPARAMTERS" );
    return( ( void *) &failure);
  } else {
    syslog(LOG_DEBUG, "onewire_hd: max_sens_meas: %d", max_sens_meas);
  }

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
        syslog( LOG_DEBUG, "onewire_hd: 1-wire device: serialnum: %s: "
          "familycode: %s", echo_serialnum(serialnum), 
                            echo_familycode( serialnum));

        /* DS2438 */
        if ( strncmp(echo_familycode(serialnum), "26",2) == 0 ) {

          /* read DS2438 VSENS */
          vsens = ReadVsens( 0, VSENS, serialnum, port);
          if ( ( err  = get_onewireinfo( "VSENS+", echo_serialnum( serialnum), 
                            &ssdp, onewirestation.config.dbtype)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS2438(VSENS+): "
             "ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            syslog(LOG_DEBUG, "onewire_hd: call to addmdat w/ mtime: %f "
              "and mval: %f\n", mtime, vsens);
            addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, vsens);
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem "
              "relation sensorname, parametername devicetyp: "
              "check database setup of serialnum: %s", 
              echo_serialnum(serialnum));
          }

          /* read DS2438 VAD */
          svdd = 0;
          SetupAtoD( portnum, svdd, serialnum);
          vad = ReadAtoD( portnum, svdd, serialnum);
          if ( ( err  = get_onewireinfo( "VAD", echo_serialnum( serialnum), 
                           &ssdp, onewirestation.config.dbtype)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS2438(VAD): ssdp.sensor_meas_no: "
              "%d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, vad);
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem "
              "relation sensorname, parametername devicetyp: "
              "check database setup of serialnum: %s", 
              echo_serialnum(serialnum));
          }

          /* read DS2438 VDD */
          svdd = 1;
          SetupAtoD( portnum, svdd, serialnum);
          vdd = ReadAtoD( portnum, svdd, serialnum);

          if ( ( err = get_onewireinfo( "VDD", echo_serialnum( serialnum), 
                          &ssdp, onewirestation.config.dbtype)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: DS2438(VDD): ssdp.sensor_meas_no: %d "
              "ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, vdd);
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem "
              "relation sensorname, parametername devicetyp: "
              "check database setup of serialnum: %s",
              echo_serialnum(serialnum));
          }

          /* read DS2438 temperature */
          temp2438 = Get_Temperature( portnum, serialnum);
          if ( ( err = get_onewireinfo( "Temperature", 
                          echo_serialnum( serialnum), 
                          &ssdp, onewirestation.config.dbtype)) == 0 ) {
            syslog(LOG_DEBUG, "onewire_hd: ds2438(Temperature): "
              "ssdp.sensor_meas_no: %d ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, temp2438);
          } else {
            syslog(LOG_ALERT, "onewire_hd: database problem "
              "relation sensorname, parametername devicetyp: "
              "check database setup of serialnum: %s", 
              echo_serialnum(serialnum));
          }

          /* humidity - derived quantity calculated from vad and vdd */
          if ( ( err = get_onewireinfo( "Humidity", echo_serialnum( serialnum), 
                          &ssdp, onewirestation.config.dbtype)) == 0 ) {
            syslog(LOG_DEBUG,
              "onewire_hd: DS2438(Humidity): ssdp.sensor_meas_no: %d "
              "ssdp.sensorname: %s ssdp.par_name: %s", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name); 
            cvad = vad;
            cvdd = vdd;
            if ( ( cvad > 0) || ( cvdd > 0)) {
              humid2438 = ( ( vad/vdd) - 0.16) * 161.29;
              humid2438 = humid2438 / ( 1.0546 - ( 0.00216 * temp2438));
	      addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, humid2438);
              syslog(LOG_DEBUG,"onewire_hd: humidity sensor discovered: %f", 
                humid2438);
            } else {
              syslog(LOG_ALERT,"onewire_hd: humidity : "
                "negative value of VAD or VDD: skipping!");
            }
          }

          /* pressure - derived quantity calculated from vad and vdd */
          else if ( ( err = get_onewireinfo( "Pressure", 
                               echo_serialnum( serialnum), 
                               &ssdp, 
                               onewirestation.config.dbtype)) == 0 ) {
            syslog(LOG_DEBUG,
              "onewire_hd: DS2438(Pressure): ssdp.sensor_meas_no: %d "
              "ssdp.sensorname: %s ssdp.par_name: %s ssdp.gain: %f "
              "ssdp.offset: %f", 
              ssdp.sensor_meas_no, ssdp.sensorname, ssdp.par_name, 
              ssdp.gain, ssdp.offset); 
            cvad = vad;
            cvdd = vdd;
            if ( ( cvad > 0) || ( cvdd > 0)) {
              press2438 = ssdp.gain * ( vad/vdd) + ssdp.offset;
	      addmdat( &mlist_p[ssdp.sensor_meas_no], mtime, press2438);
              syslog(LOG_DEBUG,"onewire_hd: pressure sensor discovered: "
                "pressure[hPa] : %f", press2438);
            } else {
              syslog(LOG_ALERT,"onewire_hd: pressure: negative value "
                "of VAD or VDD: skipping!");
            }
          }

          else {
            syslog(LOG_ALERT,"onewire_hd: Error: sensor not configured "
              "for devicetyp DS2438. serialnum: %s", echo_serialnum(serialnum));
          }

          syslog(LOG_DEBUG, 
            "onewire_hd: %f DS2438 serialnum: %s VSENS: %f VAD: %f VDD: %f "
            "Temperature: %f\n", 
	    mtime, echo_serialnum( serialnum), vsens, vad, vdd, temp2438);
          /*
          if ( verbose == 1 ) {
            ds2438mem_dump(0, TRUE, serialnum, port);
          }
          */

        /* DS1820/DS1920 */
        } else if ( strncmp(echo_familycode(serialnum), "10",2) == 0 ) {
          if ( ( err = ReadTemperature( portnum, serialnum, &temp)) == 1 ) {
            temp10 = temp;
            syslog(LOG_DEBUG, "onewire_hd: %f DS1820/DS1920 serialnum: %s "
              "Temperature: %f",
	      mtime, echo_serialnum(serialnum),temp10); 
          } else {
            syslog(LOG_ALERT, 
	      "onewire_hd: %f DS1820/DS1920 serialnum: %s "
              "Temperature conversion error", 
	      mtime, echo_serialnum( serialnum)); 
          }

          if ( ( err = get_onewireinfo( "Temperature", 
                          echo_serialnum(serialnum), 
                          &ssdp, 
                          onewirestation.config.dbtype)) == 0 ) {
            syslog(LOG_DEBUG, 
              "onewire_hd: DS1820/DS1920 (Temperature): ssdp.sensor_meas_no: %d "
              "ssdp.sensorname: %s ssdp.par_name: %s", 
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
          "onewire_hd: number of 1-Wire devices changed from %d to %d", 
          currsens, numsens);  
      }
      currsens = numsens;
      syslog(LOG_DEBUG,"onewire_hd: onewire loop: mcycle: "
        "%d %f %f %f %f %f %f %f", 
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
  if ( isdefined_sqlite("onewirestation") == TRUE ) {
    sqlite3_close( onewiredb);
  } else if ( isdefined_pgsql("onewirestation") == TRUE ) {
    PQfinish(pg_conn);
  } 

  /* release the 1-Wire Net */
  owRelease( portnum);
  syslog( LOG_INFO, "onewire_hd: closing port: %s\n", port);
  return( ( void *) &success);
}
