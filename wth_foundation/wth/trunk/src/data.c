/* data.c

   generic data handling independent of database specific routines

   $Id$
   $Revision$

   Copyright (C) 2012 Volker Jahns <volker@thalreit.de>

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
  addsdat

  adding status data in linked list slist
*/
int 
addsdat( struct sset ** slist_ref, 
         double stime, 
         long unsigned int sval) 
{
  struct sset *stat_set;
  if ( ( stat_set = ( struct sset *)malloc ( sizeof ( struct sset))) == NULL ) {
    return 1;
  } else {
    stat_set->stime = stime;
    stat_set->sval  = sval;
    stat_set->next  = *slist_ref;
    *slist_ref = stat_set;
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
  rstsdat

  delete linked list slist_p and set the head pointer to NULL
*/
void 
rstsdat( struct sset ** slist_ref) 
{
  struct sset * current = *slist_ref;
  struct sset * next;

  while ( current != NULL ) {
    next = current->next;
    free(current);
    current = next;
  }  
  *slist_ref = NULL;
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

void 
prtsdat( struct sset *slist_p) 
{
  if ( slist_p != NULL) {
    prtsdat(slist_p->next);
    syslog(LOG_DEBUG, "prtsdat: status_set->stime: %f, "
                      "status_set->sval: %lu\n", 
	              slist_p->stime, slist_p->sval);
  }
}

/*
  avgmdat

  average data in linked list and write to database
*/
void 
avgmdat( struct mset ** mlist_ref, 
         int sens_meas_no,
         int stationtype,
         int dbtype) 
{
  struct mset *llist_p = *mlist_ref;
  int count = 0;
  double avgtime = 0;
  double avgval  = 0;
  long unsigned int avgdate = 0;

  while ( llist_p != NULL) {
    count++;
    avgtime = avgtime + llist_p->mtime;
    avgval = avgval + llist_p->mval;
    llist_p = llist_p->next;
  }

  if ( count != 0) {
    avgtime = avgtime / count;
    avgval = avgval / count;
    avgdate = (long unsigned int) avgtime;
    syslog( LOG_DEBUG, 
	    "avgmdat: sens_meas_no: %d, avgtime: %f, avgdate: %lu, "
            "avgval: %f, "
            "number: %d", 
	    sens_meas_no, avgtime, avgdate, avgval, count); 
    if ( dbtype == SQLITE) {
      sqlite_datadbn( avgtime, sens_meas_no, avgval, stationtype);
    } else if ( dbtype == POSTGRESQL) {
      pg_datadb( avgtime, sens_meas_no, avgval, pg_conn);
    }
  }
}

/*
  avgsdat

  average status data in linked list and write to database
*/
void 
avgsdat( struct sset ** slist_ref, 
         int sens_flag_no,
         int stationtype,
         int dbtype) 
{
  struct sset *llist_p = *slist_ref;
  int count = 0;
  double avgtime = 0;
  double avgsval = 0;
  long unsigned int avgval  = 0;
  long unsigned int avgdate = 0;

  while ( llist_p != NULL) {
    count++;
    avgtime = avgtime + llist_p->stime;
    avgsval = avgsval + llist_p->sval;
    llist_p = llist_p->next;
  }

  if ( count != 0) {
    avgtime = avgtime / count;
    avgsval = avgsval  / count;
    avgdate = (long unsigned int) avgtime;
    avgval  = (long) (avgsval + 0.5);
    syslog( LOG_DEBUG, 
	    "avgsdat: sens_flag_no: %d, avgtime: %f, avgdate: %lu, "
            "avgval: %lu, "
            "number: %d", 
	    sens_flag_no, avgtime, avgdate, avgval, count); 
    if ( dbtype == SQLITE) {
      sqlite_statdbn( avgdate, sens_flag_no, avgval, stationtype);
    } else if ( dbtype == POSTGRESQL) {
      pg_datadb( avgdate, sens_flag_no, avgval, pg_conn);
    }
  }
}


int
maxsensmeas( int dbtype) 
{
  int p_maxsensmeas = -1;

  if (  dbtype == SQLITE) {
    p_maxsensmeas = sqlite_maxsensmeas( onewiredb);
  } else if ( dbtype == POSTGRESQL) {
    p_maxsensmeas = pg_maxsensmeas( pg_conn);
  }
  return(p_maxsensmeas);
}



sensdevpar_t
get_sensorparams( char * sensorname,  char * parametername,
                  int    stationtype, int    dbtype) 
{
  sensdevpar_t lsenspar;

  switch(dbtype) {
    case SQLITE:
      syslog(LOG_DEBUG, "get_sensorparams: dbtype is SQLITE\n");
      lsenspar = sqlite_get_sensorparams( sensorname, 
                                          parametername, 
                                          stationtype);
      break;
    default:
      syslog(LOG_ALERT, "get_sensorparams: unknown dbtype\n");
      lsenspar.sensor_meas_no = -1;
  }

  return(lsenspar);
}

sensflag_t
get_sensorflags( char * sensorname, char * flagname,
                  int  stationtype, int    dbtype) 
{
  sensflag_t lsensflag;

  switch(dbtype) {
    case SQLITE:
      syslog(LOG_DEBUG, "get_sensorflags: dbtype is SQLITE\n");
      lsensflag = sqlite_get_sensorflags( sensorname, 
                                         flagname, 
                                         stationtype);
      break;
    default:
      syslog(LOG_ALERT, "get_sensorflags: unknown dbtype\n");
      lsensflag.sensor_flag_no = -1;
  }

  return(lsensflag);
}


int measvaln_db( char *sensorname, char *parametername,
                 int stationtype, int dbtype,  
                 double mtime, float mval) 
{
  static struct mset *mlist_p[MAXSENSMEAS];
  static int cycleno[MAXSENSMEAS];
  sensdevpar_t sensorparams;
  int sensor_meas_no;
  int mcycle;

  /*
  if ( stationtype == ONEWIRE ) {
    mcycle = onewirestation.config.mcycle;
  }
  */
  switch(stationtype) {
    case UMETER:
      mcycle = umeterstation.config.mcycle;
      break;
    case ONEWIRE:
      mcycle = onewirestation.config.mcycle;
      break;
    case WMR9X8:
      mcycle = wmr9x8station.config.mcycle;
      break;
    case WS2000:
      mcycle = ws2000station.config.mcycle;
      break;
    case PCWSR:
      mcycle = pcwsrstation.config.mcycle;
      break;
    default:
      syslog(LOG_DEBUG, "measval_hd: unknown stationtype\n");
      return(1);
  }



  sensorparams = get_sensorparams( sensorname, parametername,
                                   stationtype, dbtype);
  sensor_meas_no = sensorparams.sensor_meas_no; 
  if ( sensor_meas_no == -1) {
    syslog(LOG_ALERT, "measvaln_db: configuration problem: "
                      "sensor_meas_no undefined");
    return(1);
  }
  syslog(LOG_DEBUG, "measvaln_db: sensorparams.sensor_meas_no: %d\n", 
                    sensorparams.sensor_meas_no);

  syslog(LOG_DEBUG, "measvaln_db: sensor_meas_no: %d mtime: %f mval: %f\n", 
         sensor_meas_no,
         mtime,
         mval);
  syslog(LOG_DEBUG, "measvaln_db: cycleno[%d]: %d mcycle: %d\n",
         sensor_meas_no, 
         cycleno[sensor_meas_no],
         mcycle);
  prtmdat( mlist_p[sensor_meas_no]);
  if ( cycleno[sensor_meas_no] < mcycle ) {
    syslog(LOG_DEBUG, "measvaln_db: cycleno < mcycle: adding data\n");
    addmdat( &mlist_p[sensor_meas_no], mtime, mval);
    cycleno[sensor_meas_no]++;
  } else {
    syslog(LOG_DEBUG, "measvaln_db: cycleno >= mcycle: averaging data\n");
    avgmdat( &mlist_p[sensor_meas_no], sensor_meas_no, stationtype, dbtype);
    rstmdat( &mlist_p[sensor_meas_no]);
    addmdat( &mlist_p[sensor_meas_no], mtime, mval);
    cycleno[sensor_meas_no] = 1;
  }
  return(0);

}


int measval_hd(char * sensorname, char *parametername, 
               int stationtype, int dbtype,
               double mtime, float mval)
{
  int err = 0;

  if ( stationtype == ONEWIRE ) {
    syslog(LOG_DEBUG, "measval_hd: serialnum: %s\n",   sensorname);
  } else {
    syslog(LOG_DEBUG, "measval_hd: sensorname: %s\n",  sensorname);
  }

  syslog(LOG_DEBUG, "measval_hd: parametername: %s\n", parametername);
  switch(dbtype) {
    case SQLITE:
      syslog(LOG_DEBUG, "measval_hd: dbtype is SQLITE\n");
      break;
    case POSTGRESQL:
      syslog(LOG_DEBUG, "measval_hd: dbtype is POSTGRESQL\n");
      break;
    case MYSQL:
      syslog(LOG_DEBUG, "measval_hd: dbtype is MYSQL\n");
      break;
    default:
      syslog(LOG_ALERT, "measval_hd: unknown dbtype\n");
  }

  switch(stationtype) {
    case UMETER:
      syslog(LOG_DEBUG, "measval_hd: stationtype is UMETER\n");
      break;
    case ONEWIRE:
      syslog(LOG_DEBUG, "measval_hd: stationtype is ONEWIRE\n");
      break;
    case WMR9X8:
      syslog(LOG_DEBUG, "measval_hd: stationtype is WMR9X8\n");
      break;
    case WS2000:
      syslog(LOG_DEBUG, "measval_hd: stationtype is WS2000\n");
      break;
    case PCWSR:
      syslog(LOG_DEBUG, "measval_hd: stationtype is PCWSR\n");
      break;
    default:
      syslog(LOG_DEBUG, "measval_hd: unknown stationtype\n");
  }

  syslog(LOG_DEBUG, "measval_hd: mtime: %f mval: %f\n", mtime, mval);

  err = measvaln_db( sensorname, parametername,
                     stationtype, dbtype,
                     mtime, mval);

  return(err);
}


int statvaln_db( char *sensorname, char *flagname,
                 int stationtype, int dbtype,  
                 double stime, long unsigned int sval) 
{
  static struct sset *slist_p[MAXSENSMEAS];
  static int cycleno[MAXSENSMEAS];
  sensflag_t sensorflags;
  int sensor_flag_no;
  int scycle;

  switch(stationtype) {
    case UMETER:
      scycle = umeterstation.config.mcycle;
      break;
    case ONEWIRE:
      scycle = onewirestation.config.mcycle;
      break;
    case WMR9X8:
      scycle = wmr9x8station.config.mcycle;
      break;
    case WS2000:
      scycle = ws2000station.config.mcycle;
      break;
    case PCWSR:
      scycle = pcwsrstation.config.mcycle;
      break;
    default:
      syslog(LOG_DEBUG, "statval_hd: unknown stationtype\n");
      return(1);
  }

  sensorflags = get_sensorflags( sensorname, flagname,
                                stationtype, dbtype);
  sensor_flag_no = sensorflags.sensor_flag_no; 
  if ( sensor_flag_no == -1) {
    syslog(LOG_ALERT, "statvaln_db: configuration problem: "
                      "sensor_flag_no undefined");
    return(1);
  }
  syslog(LOG_DEBUG,"statvaln_db: sensorflags.sensor_flag_no: %d\n", 
                   sensorflags.sensor_flag_no);

  syslog(LOG_DEBUG,"statvaln_db: sensor_flag_no: %d stime: %f "
         "sval: %lu\n", 
         sensor_flag_no,
         stime,
         sval);
  syslog(LOG_DEBUG, "statvaln_db: cycleno[%d]: %d scycle: %d\n",
         sensor_flag_no, 
         cycleno[sensor_flag_no],
         scycle);
  prtsdat( slist_p[sensor_flag_no]);
  if ( cycleno[sensor_flag_no] < scycle ) {
    syslog(LOG_DEBUG, 
      "statvaln_db: cycleno < scycle: adding data\n");
    addsdat( &slist_p[sensor_flag_no], stime, sval);
    cycleno[sensor_flag_no]++;
  } else {
    syslog(LOG_DEBUG, 
      "statvaln_db: cycleno >= scycle: averaging data\n");
    avgsdat( &slist_p[sensor_flag_no], 
             sensor_flag_no, 
             stationtype, 
             dbtype);
    rstsdat( &slist_p[sensor_flag_no]);
    addsdat( &slist_p[sensor_flag_no], stime, sval);
    cycleno[sensor_flag_no] = 1;
  }
  return(0);

}

/*
  statval_hd -  handle status values of weatherstation

 */
int statval_hd(char * sensorname, char *flagname, 
               int stationtype, int dbtype,
               double stime, long unsigned int sval)
{
  int err = 0;

  if ( stationtype == ONEWIRE ) {
    syslog(LOG_DEBUG, "statval_hd: serialnum: %s\n",   sensorname);
  } else {
    syslog(LOG_DEBUG, "statval_hd: sensorname: %s\n",  sensorname);
  }

  syslog(LOG_DEBUG, "statval_hd: flagname: %s\n", flagname);
  switch(dbtype) {
    case SQLITE:
      syslog(LOG_DEBUG, "statval_hd: dbtype is SQLITE\n");
      break;
    case POSTGRESQL:
      syslog(LOG_DEBUG, "statval_hd: dbtype is POSTGRESQL\n");
      break;
    case MYSQL:
      syslog(LOG_DEBUG, "statval_hd: dbtype is MYSQL\n");
      break;
    default:
      syslog(LOG_ALERT, "statval_hd: unknown dbtype\n");
  }

  switch(stationtype) {
    case UMETER:
      syslog(LOG_DEBUG, "statval_hd: stationtype is UMETER\n");
      break;
    case ONEWIRE:
      syslog(LOG_DEBUG, "statval_hd: stationtype is ONEWIRE\n");
      break;
    case WMR9X8:
      syslog(LOG_DEBUG, "statval_hd: stationtype is WMR9X8\n");
      break;
    case WS2000:
      syslog(LOG_DEBUG, "statval_hd: stationtype is WS2000\n");
      break;
    case PCWSR:
      syslog(LOG_DEBUG, "statval_hd: stationtype is PCWSR\n");
      break;
    default:
      syslog(LOG_DEBUG, "statval_hd: unknown stationtype\n");
  }

  syslog(LOG_DEBUG, "statval_hd: stime: %f sval: %lu\n", stime, sval);

  err = statvaln_db( sensorname, flagname,
                     stationtype, dbtype,
                     stime, sval);

  return(err);
}

