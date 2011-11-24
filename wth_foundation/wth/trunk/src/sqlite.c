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
  datadb - insert measured data values 
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
  stat_ws2000db - insert status values of WS2000 sensors

*/
int
stat_ws2000db( int sensor_status[], time_t statusset_date, sqlite3 *ws2000db) 
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
  new_ws2000db - insert new flag values

  new flag is read which each datarecord read command ( 1 2)
  status value is read with status command ( 5)

  there is a small time offset as both commands occur at different time
  new flag is added to table sensorstatus with the same time value as
  the sensorstatus value has been added with the status command before
  using SQL update

*/
int
new_ws2000db( long statusset_date, int sensor_no, int new_flag, sqlite3 *ws2000db) 
{
  int err = 0;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  snprintf(query, querylen, 
	   "UPDATE sensorstatus SET new_flag = %d WHERE sensor_no = %d AND statusset_date =  ( select MAX(statusset_date) from sensorstatus where sensor_no = %d )",
	   new_flag, sensor_no, sensor_no); 
  syslog(LOG_DEBUG, "new_ws2000db: query: %s", query);
  err = sqlite3_exec( ws2000db, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_ALERT,
	   "new_ws2000db: error: insert sensor status: err: %d", err);
  }

  return(err);
}


/*
  get_onewireinfo - 
  read
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
get_onewireinfo( char *parname, char *serialnum, sensdevpar_t *ssdp, sqlite3 *wthdb)
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
  is_ws2000sens
  
  check if status value of sensor is not equal to zero 
  only works for WS2000 weatherstation 
  
*/
int
is_ws2000sens( int sensor_no, sqlite3 *ws2000db)
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
