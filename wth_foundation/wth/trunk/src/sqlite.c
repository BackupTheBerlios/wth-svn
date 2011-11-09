/*
  sqlite.c

  read and write wth data to sqlite database

  $Id$
  $Revision$

  Copyright (C) Volker Jahns <volker@thalreit.de>

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
#include <sys/resource.h>

/*
  datadb - insert measured values 
           for use in WS2000, PCWSR and 1-wire database

*/
int
datadb( long dataset_date, int sensor_meas_no, float meas_value,
  sqlite3 *wthdb )
{
  int err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  /* insert data values */
  snprintf(query, querylen, 
    "INSERT INTO sensordata VALUES ( NULL, %lu, %d, %f)",
    dataset_date, sensor_meas_no, meas_value); 
  err = sqlite3_exec( wthdb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_DEBUG,
      "datadb: error: insert sensor data: err: %d", err);
  }
  syslog( LOG_DEBUG, "datadb: INSERT query: %s", query);

  /* insert date when last data has been updated */
  snprintf(query, querylen, 
    "UPDATE sensorupdate SET last_update = %lu WHERE sensor_meas_no = %d",
    dataset_date, sensor_meas_no); 
  err = sqlite3_exec( wthdb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_DEBUG, 
      "datadb: error: update sensor date: err: %d", err);
  }
  syslog( LOG_DEBUG, "datadb: UPDATE query: %s", query);

  return(err);
}


/*
  statdb - insert status values

*/
int
statdb( int sensor_status[], time_t statusset_date, sqlite3 *ws2000db) 
{
  int i, err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  err = 0;
  for ( i = 1; i <= 18; i++) {
    snprintf(query, querylen, 
     "INSERT INTO sensorstatus (statusset_no, statusset_date, sensor_no, sensor_status) VALUES ( NULL, %lu, %d, %d)",
     (long unsigned int) statusset_date, i, sensor_status[i]); 
    err = sqlite3_exec( ws2000db, query, NULL, NULL, NULL);
    if ( err) { 
      syslog(LOG_ALERT,
        "statdb: error: insert sensor status: err: %d", err);
    }

  }

  return(err);
}


/*
  newdb - insert new flag values

  new flag is read which each datarecord read command ( 1 2)
  status value is read with status command ( 5)

  there is a small time offset as both commands occur at different time
  new flag is added to table sensorstatus with the same time value as
  the sensorstatus value has been added with the status command before
  using SQL update

*/
int
newdb( long statusset_date, int sensor_no, int new_flag, sqlite3 *ws2000db) 
{
  int err = 0;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  snprintf(query, querylen, 
	   "UPDATE sensorstatus SET new_flag = %d WHERE sensor_no = %d AND statusset_date =  ( select MAX(statusset_date) from sensorstatus where sensor_no = %d )",
	   new_flag, sensor_no, sensor_no); 
  syslog(LOG_DEBUG, "newdb: query: %s", query);
  err = sqlite3_exec( ws2000db, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_ALERT,
	   "newdb: error: insert sensor status: err: %d", err);
  }

  return(err);
}


/*
  senspardb - retrieve sensor number and parameter name
*/
int
senspardb( int sensor_meas_no, senspar_t *sspar, sqlite3 *wthdb)
{
  int err, rowcnt;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;

  snprintf(query, SBUFF, 
    "SELECT sp.sensor_no, sn.sensor_name, pn.parameter_name "
    "FROM sensorparameters AS sp, sensornames AS sn, parameternames "
    "AS pn WHERE sp.parameter_no = pn.parameter_no "
    "AND sp.sensor_no = sn.sensor_no AND sp.sensor_meas_no = %d", 
    sensor_meas_no);

  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "Error: select parametername: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(1);
  }

  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "senspardb  : sensor_meas_no: %d : sensor_no: %d",
	   sensor_meas_no, sqlite3_column_int(qcomp, 0));
    syslog(LOG_DEBUG, "senspardb  : sensorname: %s : parametername: %s",
	   (char *)sqlite3_column_text(qcomp, 1), (char *)sqlite3_column_text(qcomp, 2));
    sspar->sensor_no   = sqlite3_column_int(qcomp,0); 
    sspar->sensor_name = malloc(sizeof(char)*TBUFF+1); 
    sspar->par_name    = malloc(sizeof(char)*TBUFF+1);
    strncpy(sspar->sensor_name, (char *)sqlite3_column_text(qcomp,1), TBUFF);  
    strncpy(sspar->par_name, (char *)sqlite3_column_text(qcomp,2), TBUFF);  
    rowcnt++;
  }
  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "Error: select parametername: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(1);
  }
  if ( rowcnt == 0) {
    syslog( LOG_ALERT, "Error: no configuration data in database");
    return(1);
  }
  return(0);
}


char *
get_dbfile( char *wstation) {
  int err;
  static char dbfile[TBUFF+1];

  if ( ( err = strncmp( wstation,"ws2001",5)) == 0 ) {
    strncpy( dbfile, ws2000station.config.dbfile, strlen(ws2000station.config.dbfile));
  } else if ( ( err = strncmp( wstation, "pcwsr", 5)) == 0 ) {
    strncpy( dbfile, pcwsrstation.config.dbfile, strlen(pcwsrstation.config.dbfile));
  } else if ( ( err = strncmp( wstation, "onewire", 7)) == 0 ) {
    strncpy( dbfile, onewirestation.config.dbfile, strlen(onewirestation.config.dbfile));
  } else if ( ( err = strncmp( wstation, "wmr9x8", 6)) == 0 ) {
    strncpy( dbfile, wmr9x8station.config.dbfile, strlen(wmr9x8station.config.dbfile));
  } else if ( ( err = strncmp( wstation, "umeter", 6)) == 0 ) {
    strncpy( dbfile, umeterstation.config.dbfile, strlen(umeterstation.config.dbfile));
  } else {
    strncpy(dbfile, "\0", 1); 
  }

  return(dbfile);
}

/*
  get_sensor_meas_no

  retrieve sensor_meas_no from sensorname and parametername ( and serialnum)

  input parameters
    sensor_name     - required
    parameter_name  - required
    serialnum       - optional, necessary for 1-wire device

  output parameters
    sensor_meas_no  - value of database
                    - -255 if error has occurred
*/
int
get_sensor_meas_no( char *sensor_name, char *parameter_name, char *serialnum,
  char *wstation) 
{
  int p_sensor_meas_no = -255;
  int err, rowcnt;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;
  sqlite3 *wthdb;
  char *dbfile;

  if ( parameter_name == NULL ) {
    syslog(LOG_ERR, 
      "get_sensor_meas_no: parameter_name must not be NULL");
    return(p_sensor_meas_no);
  }

  dbfile = get_dbfile( wstation);
  err = sqlite3_open( dbfile, &wthdb);
  if ( err) {
    syslog( LOG_ALERT, "get_sensor_meas_no: failed to open database %s.\n",
          ws2000station.config.dbfile);
    return (p_sensor_meas_no);
  } 

  /* only 1-wire devices have serialnum */
  if ( serialnum != NULL ) {
    snprintf(query, SBUFF, 
      "SELECT sp.sensor_meas_no "
      "FROM sensorparameters AS sp, sensornames AS sn, parameternames AS pn, devicetyp AS dt "
      "WHERE sp.parameter_no = pn.parameter_no "
      "AND sp.sensor_no = sn.sensor_no "
      "AND sp.device_no = dt.device_no "
      "AND dt.serialnum = '%s' " 
      "AND pn.parameter_name = '%s' "
      "AND sn.sensor_name = '%s' ", 
    serialnum, parameter_name, sensor_name);
  /* handle other weatherstation devices */
  } else { 
    snprintf(query, SBUFF, 
      "SELECT sp.sensor_meas_no "
      "FROM sensorparameters AS sp, sensornames AS sn, parameternames AS pn "
      "WHERE sp.parameter_no = pn.parameter_no "
      "AND sp.sensor_no = sn.sensor_no "
      "AND pn.parameter_name = '%s' "
      "AND sn.sensor_name = '%s' ", 
    parameter_name, sensor_name);
  }

  syslog(LOG_DEBUG, "get_sensor_meas_no: sensor_meas_no SELECT: %s", query);
  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
      "get_sensor_meas_no: error: select sensor_meas_no: err: %d", err);
    return(p_sensor_meas_no);
  }

  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    p_sensor_meas_no = sqlite3_column_int(qcomp, 0);
    rowcnt++;
  }
  if ( rowcnt != 1) {
    syslog( LOG_ALERT, 
      "get_sensor_meas_no: error: configuration data in database");
    p_sensor_meas_no = -255; return(p_sensor_meas_no);
  }
  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT, "get_sensor_meas_no: err: %d", err);
    p_sensor_meas_no = -255; return(p_sensor_meas_no);
  }
  sqlite3_close( wthdb);
  return(p_sensor_meas_no);
}

/*
  get_sensorinfo

  retrieve sensorinfo datastructure

  needs 
    sensor_meas_no
    wstation

  returns 
    struct snesorinfo_t

*/
sensorinfo_t 
get_sensorinfo( int sensor_meas_no, char *wstation)
{
  static sensorinfo_t devinfo;
  int err, rowcnt;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;
  sqlite3 *wthdb;
  char *dbfile;

  dbfile = get_dbfile( wstation);
  err = sqlite3_open( dbfile, &wthdb);
  if ( err) {
    syslog( LOG_ALERT, "get_sensor_meas_no: failed to open database %s.\n",
          ws2000station.config.dbfile);
    return (devinfo);
  } 

  if ( strncmp( wstation, "onewire", 6) == 0 ) {
    snprintf(query, SBUFF, 
      "SELECT sp.sensor_no, sn.sensor_name, "
      "pn.parameter_no, pn.parameter_name, pn.gain, pn.offset, "
      "dt.device_no, dt.devicetyp, dt.familycode, dt.serialnum "
      "FROM sensorparameters AS sp, sensornames AS sn, parameternames "
      "AS pn, devicetyp AS dt "
      "WHERE sp.parameter_no = pn.parameter_no "
      "AND sp.sensor_no = sn.sensor_no " 
      "AND sp.device_no = dt.device_no "
      "AND sp.sensor_meas_no = %d", 
      sensor_meas_no);
  } else {
    snprintf(query, SBUFF, 
      "SELECT sp.sensor_no, sn.sensor_name, pn.parameter_no, pn.parameter_name, "
      "pn.gain, pn.offset "
      "FROM sensorparameters AS sp, sensornames AS sn, parameternames "
      "AS pn WHERE sp.parameter_no = pn.parameter_no "
      "AND sp.sensor_no = sn.sensor_no " 
      "AND sp.sensor_meas_no = %d", 
      sensor_meas_no);
  }
  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "Error: select parametername: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(devinfo);
  }

  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "senspardb  : sensor_meas_no: %d : sensor_no: %d",
	   sensor_meas_no, sqlite3_column_int(qcomp, 0));
    syslog(LOG_DEBUG, "senspardb  : sensorname: %s : parametername: %s",
	   (char *)sqlite3_column_text(qcomp, 1), (char *)sqlite3_column_text(qcomp, 2));
    devinfo.sensor_no       = sqlite3_column_int(qcomp,0); 
    strncpy(devinfo.sensor_name, (char *)sqlite3_column_text(qcomp,1), TBUFF);  
    devinfo.parameter_no    = sqlite3_column_int(qcomp, 2);
    strncpy(devinfo.parameter_name, (char *)sqlite3_column_text(qcomp,3), TBUFF);  
    devinfo.sensor_meas_no  = sensor_meas_no;
    devinfo.gain            = (float)sqlite3_column_double(qcomp,4);
    devinfo.offset          = (float)sqlite3_column_double(qcomp,5);

    rowcnt++;
  }
  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "Error: select parametername: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(devinfo);
  }
  if ( rowcnt == 0) {
    syslog( LOG_ALERT, "Error: no configuration data in database");
    return(devinfo);
  }
  return(devinfo);
}

/*
  sensdevpar - retrieve
    sensor_meas_no,
    sensor_name,
    parameter_name,
    gain,
    offset, 
    sensor number, 
    devicetyp and 
    parameter name of
  1-wire device
  returns these data in structure ssdp
*/
int
sensdevpar( char *parname, char *serialnum, sensdevpar_t *ssdp, sqlite3 *wthdb)
{
  int err, rowcnt;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;

  snprintf(query, SBUFF, 
    "SELECT sp.sensor_meas_no, sn.sensor_name, "
    "pn.parameter_name, pn.offset, pn.gain, "
    "dt.devicetyp, dt.familycode, dt.serialnum "
    "FROM sensorparameters AS sp, sensornames AS sn, parameternames AS pn, devicetyp AS dt "
    "WHERE sp.parameter_no = pn.parameter_no "
    "AND sp.sensor_no = sn.sensor_no "
    "AND sp.device_no = dt.device_no "
    "AND dt.serialnum = '%s' " 
    "AND pn.parameter_name = '%s' ", 
    serialnum, parname);
  //syslog(LOG_DEBUG, "sensdevpar: sql: %s", query);

  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "sensdevpar: error: select sensdevpar: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(1);
  }

  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {

    ssdp->sensor_meas_no = sqlite3_column_int(qcomp, 0);
    strncpy(ssdp->sensorname, (char *)sqlite3_column_text(qcomp,1), TBUFF);  
    strncpy(ssdp->par_name, (char *)sqlite3_column_text(qcomp,2), TBUFF);
    ssdp->offset = sqlite3_column_double(qcomp,3);
    ssdp->gain   = sqlite3_column_double(qcomp,4);
    strncpy(ssdp->devicetyp, (char *)sqlite3_column_text(qcomp,5), TBUFF);  
    strncpy(ssdp->familycode, (char *)sqlite3_column_text(qcomp,6), TBUFF);  
    strncpy(ssdp->serialnum, (char *)sqlite3_column_text(qcomp,7), TBUFF);  
    rowcnt++;
  }
  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "sensdevpar: error: select parametername: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(1);
  }
  if ( rowcnt == 0) {
    //syslog( LOG_ALERT, "sensdevpar: error: no configuration data in database");
    return(1);
  }
  return(0);
}


/*
  writedb

  writing ws2000 data to sqlite database

 */
int
writedb( int sensor_no, int nval, int sensor_meas_no[], time_t dataset_date, 
         float meas_value[], sqlite3 *ws2000db ) {
  int i, err;
  
  err = 0;        
  for ( i = 0; i < nval; i++) {
    err = datadb( dataset_date, sensor_meas_no[i], meas_value[i], ws2000db);
  }

  return(err);
}

/*
  read sensor data
*/
char *
readdb( char *wstation) {
  int err;
  static char rbuf[NBUFF+1];;
  char s[TBUFF+1], buf[TBUFF+1], query[SBUFF+1];
  sqlite3_stmt *qcomp;
  struct tm *tm;
  time_t meastim;

  //snprintf(rbuf, NBUFF, "");
  memset(rbuf, 0, NBUFF);
  /* ---------------------------------- */
  /* read data of ws2000 weatherstation */
  /* ---------------------------------- */
  if ( ( err = strncmp( wstation,"ws2000",5)) == 0 ) {
    /* open sqlite db file */
    err = sqlite3_open( ws2000station.config.dbfile, &ws2000db);
    if ( err) {
      syslog( LOG_ALERT, "readdb: failed to open database %s.\n", 
	    ws2000station.config.dbfile);
      return (NULL);
    } else {
      syslog(LOG_DEBUG, "readdb: sqlite3_open: OK\n");
    }

    snprintf( query, SBUFF, 
      "SELECT DISTINCT sensornames.sensor_name,parameternames.parameter_name, "
      "sensorupdate.last_update, sensordata.meas_value, "
      "parameternames.unit "
      "FROM " 
        "sensorupdate, sensordata,sensornames,sensorparameters,parameternames "
      "WHERE sensorupdate.last_update = sensordata.dataset_date "
      "AND "
        "sensorupdate.sensor_meas_no = sensordata.sensor_meas_no "
      "AND "
        "sensornames.sensor_no = sensorparameters.sensor_no "
      "AND "
        "sensorupdate.sensor_meas_no = sensorparameters.sensor_meas_no "
      "AND "
        "parameternames.parameter_no = sensorparameters.parameter_no"
      );
    err = sqlite3_prepare( ws2000db, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF, 
        "Error: readdb: select ws2000 data: err: %d", err);
      syslog( LOG_ALERT, "%s", rbuf);
      snprintf( rbuf, TBUFF, "database error: please check installation\n");
      return(rbuf);
    }

    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      meastim = (time_t )sqlite3_column_int( qcomp, 2); 
      tm = gmtime(&meastim);
      strftime(buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
      snprintf(s, SBUFF, "%-12s %-18s %8.2f %-8s %-12s\n",
	(char *)sqlite3_column_text( qcomp, 0),
        (char *)sqlite3_column_text( qcomp, 1),
        (float)sqlite3_column_double( qcomp, 3),
        (char *)sqlite3_column_text( qcomp, 4),
        buf
      );
      strncat(rbuf,s,strlen(s));
    }
    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      snprintf(rbuf, TBUFF, 
        "Error: readdb: select parametername: err: %d : sqlite_errmsg: %s\n", 
	err, sqlite3_errmsg(ws2000db));
      syslog( LOG_ALERT, "%s", rbuf);
      snprintf(rbuf, TBUFF, "database error: please check installation\n");
      return(rbuf);
    }

    /* cleanup and close */
    sqlite3_close( ws2000db);
    syslog(LOG_DEBUG,"readdb: sqlite3_close ws2000db done\n");

  /* --------------------------------- */
  /* read data of pcwsr weatherstation */      
  /* --------------------------------- */
  } else if ( ( err = strncmp( wstation,"pcwsr",5)) == 0 ) {
    printf("readdb: pcwsr : %s\n", wstation);
    /* open sqlite db file */
    err = sqlite3_open( pcwsrstation.config.dbfile, &pcwsrdb);
    syslog(LOG_DEBUG, 
	 "readdb: sqlite3_open %s return value: %d", 
	 pcwsrstation.config.dbfile,
	 err);
    if ( err) {
      syslog( LOG_ALERT, "readdb: failed to open database %s.", 
	    pcwsrstation.config.dbfile);
      return (NULL);
    } else {
      syslog(LOG_DEBUG, "readdb: sqlite3_open: OK\n");
    }

    snprintf( query, SBUFF, 
      "SELECT DISTINCT sensornames.sensor_name, sensornames.sensor_no, "
      "parameternames.parameter_name, "
      "sensorupdate.last_update, sensordata.meas_value, "
      "parameternames.unit "
      "FROM " 
        "sensorupdate, sensordata,sensornames,sensorparameters,parameternames "
      "WHERE sensorupdate.last_update = sensordata.dataset_date "
      "AND "
        "sensorupdate.sensor_meas_no = sensordata.sensor_meas_no "
      "AND "
        "sensornames.sensor_no = sensorparameters.sensor_no "
      "AND "
        "sensorupdate.sensor_meas_no = sensorparameters.sensor_meas_no "
      "AND "
        "parameternames.parameter_no = sensorparameters.parameter_no"
      );
    err = sqlite3_prepare( pcwsrdb, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF,
        "Error: readdb: select pcwsr data: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(pcwsrdb));
      syslog( LOG_ALERT, "%s", rbuf);
      snprintf( rbuf, TBUFF, "database error: please check installation\n");
      return(rbuf);
    }

    snprintf( s, TBUFF, 
      "    sensorname parameter             value unit     dataset date\n"
      "-------------- -----------   ------------- -------- -------------------------\n");
    strncat(rbuf, s, strlen(s));
    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      meastim = (time_t )sqlite3_column_int( qcomp, 3); 
      tm = gmtime(&meastim);
      strftime(buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
      snprintf( s, TBUFF, "%15s%d %-18s %8.2f %-8s %-12s\n",
	(char *)sqlite3_column_text( qcomp, 0),
        (int)sqlite3_column_int( qcomp, 1),
        (char *)sqlite3_column_text( qcomp, 2),
        (float)sqlite3_column_double( qcomp, 4),
        (char *)sqlite3_column_text( qcomp, 5),
        buf
      );
      strncat(rbuf,s,strlen(s));
    }
    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF, 
        "Error: readdb: select parametername: err: %d : sqlite_errmsg: %s\n", 
	err, sqlite3_errmsg(pcwsrdb));
      syslog( LOG_ALERT, "%s", rbuf);
      snprintf( rbuf, TBUFF, "database error: please check installation");
      return(rbuf);
    }

    /* cleanup and close */
    sqlite3_close( pcwsrdb);
    syslog(LOG_DEBUG,"readdb: sqlite3_close pcwsrdb done\n");      

  /* ----------------------------------- */
  /* read data of onewire weatherstation */
  /* ----------------------------------- */
  } else if ( ( err = strncmp( wstation,"onewire",5)) == 0 ) {

    printf("readdb: onewire : %s\n", wstation);
    /* open sqlite db file */
    err = sqlite3_open( onewirestation.config.dbfile, &onewiredb);
    syslog(LOG_DEBUG, 
	 "readdb: sqlite3_open %s return value: %d", 
	 onewirestation.config.dbfile, err);
    if ( err) {
      syslog( LOG_ALERT, "readdb: failed to open database %s.", 
	    onewirestation.config.dbfile);
      return (NULL);
    } else {
      syslog(LOG_DEBUG, "readdb: sqlite3_open: OK\n");
    }

    snprintf( query, SBUFF, 
      "SELECT DISTINCT sensorname.sensor_name, sensorname.sensor_no, "
      "parametername.parameter_name, "
      "sensorupdate.last_update, sensordata.meas_value, "
      "parametername.unit "
      "FROM " 
        "sensorupdate, sensordata,sensorname,sensordevparameters,parametername "
      "WHERE sensorupdate.last_update = sensordata.dataset_date "
      "AND "
        "sensorupdate.sensor_meas_no = sensordata.sensor_meas_no "
      "AND "
        "sensorname.sensor_no = sensordevparameters.sensor_no "
      "AND "
        "sensorupdate.sensor_meas_no = sensordevparameters.sensor_meas_no "
      "AND "
        "parametername.parameter_no = sensordevparameters.parameter_no"
      );
    err = sqlite3_prepare( onewiredb, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF,
        "Error: readdb: select pcwsr data: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(onewiredb));
      syslog( LOG_ALERT, "%s", rbuf);
      snprintf( rbuf, TBUFF, "database error: please check installation");
      return(rbuf);
    }

    snprintf( s, TBUFF, 
      "      sensorname parameter             value unit     dataset date\n"
      "---------------- -----------   ------------- -------- -------------------------\n");
    strncat(rbuf, s, strlen(s));
    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      meastim = (time_t )sqlite3_column_int( qcomp, 3); 
      tm = gmtime(&meastim);
      strftime(buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
      snprintf( s, TBUFF, " %15s %-18s %8.2f %-8s %-12s\n",
	(char *)sqlite3_column_text( qcomp, 0),
        (char *)sqlite3_column_text( qcomp, 2),
        (float)sqlite3_column_double( qcomp, 4),
        (char *)sqlite3_column_text( qcomp, 5),
        buf
      );
      strncat(rbuf,s,strlen(s));
    }

    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF, 
        "Error: readdb: select parametername: err: %d : sqlite_errmsg: %s\n", 
	err, sqlite3_errmsg(onewiredb));
      syslog( LOG_ALERT, "%s", rbuf);
      snprintf( rbuf, TBUFF, "database error: please check installation");
      return(rbuf);
    }

    /* cleanup and close */
    sqlite3_close( onewiredb);
    syslog(LOG_DEBUG,"readdb: sqlite3_close onewiredb done\n");      

  } else {
      return(NULL);
  }
  return(rbuf);
}


/*
  issens
  
  check if status value of sensor is not equal to zero 
  only works fpr WS2000 weatherstation 
  
*/
int
issens( int sensor_no, sqlite3 *ws2000db)
{
  int err;
  time_t stattim;
  int statval;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;

  snprintf(query, SBUFF, 
    "SELECT sensor_status, max(statusset_date) FROM sensorstatus WHERE sensor_no = %d",
    sensor_no);
  err = sqlite3_prepare( ws2000db, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT, "Error: issens: select ws2000 status data: err: %d", err);
    return(err);
  }

  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    statval = (int )sqlite3_column_int( qcomp, 0); 
    stattim = (time_t )sqlite3_column_int( qcomp, 1); 
  }

  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK )  {
    syslog( LOG_ALERT,  "Error: issens: sqilte3_finalize");
    return(err);
  } 

  return(statval); 
}

int
maxsensmeas( sqlite3 *onewiredb) {
  int max_sens_meas;
  int err, rowcnt;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;

  snprintf(query, SBUFF, "SELECT COUNT(*) "
    "FROM sensorparameters");
  syslog(LOG_DEBUG, "maxsensmeas: sql: %s", query);

  err = sqlite3_prepare( onewiredb, query, -1, &qcomp, 0);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "Error: select maxsensmeas: err: %d : sqlite_errmsg: %s\n",
            err, sqlite3_errmsg(onewiredb));
    return(1);
  }

  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "maxsensmeas  : max_sens_meas: %d",
           sqlite3_column_int(qcomp, 0));
    max_sens_meas = sqlite3_column_int(qcomp, 0);
    rowcnt++;
  }

  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "Error: select parametername: err: %d : sqlite_errmsg: %s\n",
            err, sqlite3_errmsg(onewiredb));
    return(1);
  }

  if ( rowcnt == 0) {
    syslog( LOG_ALERT, "Error: cant find relation sensor - device - parameter");
    return(1);
  }

  return(max_sens_meas);
}

/*

  measval_db

  ULTIMETER and WMR928 NX measurement data to sqlite database

 */
int
measval_db( char *sensorname, char *parametername, 
                   time_t dataset_date, float mval, sqlite3 *wthdb) {
  int err;
  char *errmsg;
  int rowcnt, sensor_no, parameter_no, sensor_meas_no;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;

  syslog(LOG_DEBUG, "measval_db: sensorname: %s parametername: %s", 
                                sensorname, parametername);

  /* select sensor_no */
  snprintf(query, SBUFF, 
    "SELECT sensor_no FROM sensornames WHERE sensor_name = '%s'",
    sensorname);

  syslog(LOG_DEBUG, "measval_sb: query :\'%s\'", query);

  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "measval_db: error: select sensor_no: err: %d : sqlite_errmsg: %s\n", 
            err, sqlite3_errmsg(wthdb));
    return(err);
  }
  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "measval_db : sensorname: \'%s\' sensor_no: %d", 
              sensorname, sqlite3_column_int(qcomp, 0));
    sensor_no   = sqlite3_column_int(qcomp,0); 
    rowcnt++;
  }
  if ( rowcnt == 0 )
    syslog(LOG_ALERT," no sensorname %s\n in database", sensorname);

  err = sqlite3_finalize(qcomp);

  /* select parameter_no of parametername */
  snprintf(query, SBUFF, 
    "SELECT parameter_no FROM parameternames WHERE parameter_name = '%s'",
    parametername);

  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "measval_db: error: select parameter_no of \'%s\': err: %d : sqlite_errmsg: %s\n", 
            parametername, err, sqlite3_errmsg(wthdb));
    return(err);
  }
  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "measval_db : parametername: \'%s\' parameter_no: %d", 
              parametername, sqlite3_column_int(qcomp, 0));
    parameter_no   = sqlite3_column_int(qcomp,0); 
    rowcnt++;
  }
  if ( rowcnt == 0 )
    syslog(LOG_ALERT," no parametername %s\n in database", parametername);

  err = sqlite3_finalize(qcomp);

  /* select sens_meas_no of windsensor and wind_direction */
  snprintf(query, SBUFF, 
    "SELECT sensor_meas_no FROM sensorparameters WHERE sensor_no = %d AND parameter_no = %d",
    sensor_no, parameter_no);

  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "measval_db: error: select sensor_meas_no of \'%s\' and \'%s\': err: %d : sqlite_errmsg: %s\n", 
            sensorname, parametername, err, sqlite3_errmsg(wthdb));
    return(err);
  }
  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "measval_db : sensor_meas_no: %d", sqlite3_column_int(qcomp, 0));
    sensor_meas_no   = sqlite3_column_int(qcomp,0); 
    rowcnt++;
  }
  if ( rowcnt == 0 )
    syslog(LOG_ALERT," no sensor_meas_no in database");

  err = sqlite3_finalize(qcomp);

 /* insert data values */
  snprintf(query, SBUFF, 
    "INSERT INTO sensordata VALUES ( NULL, %lu, %d, %f)",
    (long unsigned int)dataset_date, sensor_meas_no, (double)mval); 
  err = sqlite3_exec( wthdb, query, NULL, NULL, &errmsg);
  if ( err) { 
    syslog(LOG_DEBUG,
      "measval_db: error: insert sensor data: err: %d : sqlite_errmsg: %s\n", 
      err, sqlite3_errmsg(wthdb));
  }
  if ( errmsg != NULL) {
    sqlite3_free( errmsg);
    errmsg = NULL;
  }

  /* insert date when last data has been updated */
  snprintf(query, SBUFF, 
    "UPDATE sensorupdate SET last_update = %lu WHERE sensor_meas_no = %d",
    (long unsigned int)dataset_date, sensor_meas_no); 
  err = sqlite3_exec( wthdb, query, NULL, NULL, &errmsg);
  if ( err) { 
    syslog(LOG_DEBUG, 
      "measval_db: error: update sensor date: err: %d : sqlite_errmsg: %s\n", 
      err, sqlite3_errmsg(wthdb));
  }
  if ( errmsg != NULL) {
    sqlite3_free( errmsg);
    errmsg = NULL;
  }

  /* clean up and close */
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "measval_db: error database handling: err: %d : sqlite_errmsg: %s\n", 
            err, sqlite3_errmsg(wthdb));
    return(err);
  }

  return err;
}

int
statval_db( char *sensorname, char *flagname,
                 time_t statusset_date, long unsigned int sval, sqlite3 *wthdb) {
  int err;
  int sensor_no, flag_no, sensor_flag_no;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;

  /* select sensor_no */
  snprintf(query, SBUFF, 
    "SELECT sensor_no FROM sensornames WHERE sensor_name = '%s'",
    sensorname);

  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "statval_db: rror: select sensor_no: err: %d", err);
    return(err);
  }
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "statval_db : sensorname: %s sensor_no: %d", 
                                 sensorname, sqlite3_column_int(qcomp, 0));
    sensor_no   = sqlite3_column_int(qcomp,0); 
  }
  err = sqlite3_finalize(qcomp);

  /* select flag_no of flagnames */
  snprintf(query, SBUFF, 
    "SELECT flag_no FROM flagnames WHERE flag_name = '%s'",
    flagname);

  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "statval_db: error: select flag_no of %s: err: %d", flagname, err);
    return(err);
  }
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "statval_db : flagname: %s flag_no: %d", 
                                 flagname, sqlite3_column_int(qcomp, 0));
    flag_no   = sqlite3_column_int(qcomp,0); 
  }
  err = sqlite3_finalize(qcomp);

  /* select sens_flag_no of sensorname and flagname */
  snprintf(query, SBUFF, 
    "SELECT sensor_flag_no FROM sensorflags WHERE sensor_no = %d AND flag_no = %d",
    sensor_no, flag_no);

  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "statval_db: error: select sensor_flag_no of %s and %s: err: %d", 
            sensorname, flagname, err );
    return(err);
  }
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "statval_db : sensor_flag_no: %d", sqlite3_column_int(qcomp, 0));
    sensor_flag_no   = sqlite3_column_int(qcomp,0); 
  }
  err = sqlite3_finalize(qcomp);

 /* insert status values */
  snprintf(query, SBUFF, 
    "INSERT INTO statusdata VALUES ( NULL, %lu, %d, %lu)",
    (long unsigned int)statusset_date, sensor_flag_no, (long unsigned int)sval); 
  err = sqlite3_exec( wthdb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_DEBUG,
      "statval_db: error: insert status data: err: %d", err);
  }

  /* clean up and close */
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "statval_db: error database handling: err: %d", err);
    return(err);
  }

  return err;
}

/*
  use_sqlite
  check if sqlite database type is defined
  return 0 if sqlite database type is defined
  return 1 if sqlite database type is not defined

*/
int
isdefined_sqlite( void ) {
  int err;

  if ( strncmp(ws2000station.config.dbtype,"sqlite",6) == 0) {
    err = 1;
  } else { err = 0; }   
  return(err);
}
