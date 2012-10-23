/*
  sqlite.c

  read and write wth data to sqlite database

  $Id:$
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
  datadb - insert measured data values for use in 
    - 1-Wire
    - WS2000 
    - PCWSR 
  database

*/
int
sqlite_datadb( long dataset_date, int sensor_meas_no, float meas_value,
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
sqlite_stat_ws2000db( int sensor_status[], time_t statusset_date) 
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
        "sqlite_stat_ws2000db: error: insert sensor status: err: %d", err);
      syslog(LOG_ALERT,
        "sqlite_stat_ws2000db: query: \"%s\"",query);
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
sqlite_new_ws2000db( int sensor_no, int new_flag)
{
  int err = 0;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  snprintf(query, querylen, 
	   "UPDATE sensorstatus SET new_flag = %d WHERE sensor_no = %d AND statusset_date =  ( select MAX(statusset_date) from sensorstatus where sensor_no = %d )",
	   new_flag, sensor_no, sensor_no); 
  syslog(LOG_DEBUG, "sqlite_new_ws2000db: query: %s", query);
  err = sqlite3_exec( ws2000db, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_ALERT,
	   "sqlite_new_ws2000db: error: insert sensor status: err: %d", err);
  }

  return(err);
}


/*
  sqlite_writedb

  writing ws2000 data to sqlite database

 */
int
sqlite_writedb( int sensor_no, int nval, int sensor_meas_no[],
                time_t dataset_date, float meas_value[], 
                sqlite3 *ws2000db ) {
  int i, err;
  
  err = 0;        
  for ( i = 0; i < nval; i++) {
    err = sqlite_datadb( dataset_date, sensor_meas_no[i], 
                         meas_value[i], ws2000db);
  }

  return(err);
}


/*
  sqlite_is_ws2000sens
  
  check if status value of sensor is not equal to zero 
  only works for WS2000 weatherstation 
  
*/
int
sqlite_is_ws2000sens( int sensor_no)
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


sensdevpar_t
sqlite_get_sensorparams( char *sensorname, char*parametername,
                         int stationtype)
{
  int err;
  int rowcnt = 0;
  char query[SBUFF+1] = "\0";
  char serialnum[TBUFF+1] = "\0";
  sensdevpar_t sqsenspar;
  sqlite3 *sqlitedb;
  sqlite3_stmt *qcomp;

   
  switch(stationtype) {
    case ONEWIRE:
      syslog(LOG_DEBUG, "sqlite_get_sensorparams: stationtype is "
        "ONEWIRE\n");
      sqlitedb = onewiredb;
      strncpy( serialnum, sensorname, strlen( sensorname));
      snprintf(query, SBUFF,
        "SELECT sp.sensor_meas_no, sn.sensor_name, "
        "pn.param_name, pn.param_offset, pn.param_gain, "
        "dt.devicetyp, dt.familycode, dt.serialnum "
        "FROM sensorparameters AS sp, sensornames AS sn, "
        "parameternames AS pn, devicetyp AS dt "
        "WHERE sp.param_no = pn.param_no "
        "AND sp.sensor_no = sn.sensor_no "
        "AND sp.device_no = dt.device_no "
        "AND dt.serialnum = '%s' "
        "AND pn.param_name = '%s' ",
        serialnum, parametername);
      //syslog(LOG_DEBUG, "sqlite_get_sensorparams: query: %s\n", query);
      break;
    case WMR9X8:
      syslog(LOG_DEBUG, "sqlite_get_sensorparams: stationtype is "
        "WMR9X8\n");
      sqlitedb = wmr9x8db;
      snprintf(query, SBUFF,
        "SELECT sp.sensor_meas_no, sn.sensor_name, "
        "pn.param_name, pn.param_offset, pn.param_gain "
        "FROM sensorparameters AS sp, sensornames AS sn, "
        "parameternames AS pn "
        "WHERE sp.param_no = pn.param_no "
        "AND sp.sensor_no = sn.sensor_no "
        "AND sn.sensor_name = '%s' "
        "AND pn.param_name  = '%s' ",
        sensorname, parametername);
      syslog(LOG_DEBUG, "sqlite_get_sensorparams: query: %s\n", query);
      break;
    case UMETER:
      syslog(LOG_DEBUG, "sqlite_get_sensorparams: stationtype is "
        "UMETER\n");
      sqlitedb = umeterdb;
      snprintf(query, SBUFF,
        "SELECT sp.sensor_meas_no, sn.sensor_name, "
        "pn.param_name, pn.param_offset, pn.param_gain "
        "FROM sensorparameters AS sp, sensornames AS sn, "
        "parameternames AS pn "
        "WHERE sp.param_no = pn.param_no "
        "AND sp.sensor_no = sn.sensor_no "
        "AND sn.sensor_name = '%s' "
        "AND pn.param_name  = '%s' ",
        sensorname, parametername);
      syslog(LOG_DEBUG, "sqlite_get_sensorparams: query: %s\n", query);
      break;
    case WS2000:
      syslog(LOG_DEBUG, "sqlite_get_sensorparams: stationtype is "
        "WS2000\n");
      sqlitedb = ws2000db;
      snprintf(query, SBUFF,
        "SELECT sp.sensor_meas_no, sn.sensor_name, "
        "pn.param_name, pn.param_offset, pn.param_gain "
        "FROM sensorparameters AS sp, sensornames AS sn, "
        "parameternames AS pn "
        "WHERE sp.param_no = pn.param_no "
        "AND sp.sensor_no = sn.sensor_no "
        "AND sn.sensor_name = '%s' "
        "AND pn.param_name  = '%s' ",
        sensorname, parametername);
      syslog(LOG_DEBUG, "sqlite_get_sensorparams: query: %s\n", query);
      break;
    default:
      syslog(LOG_ALERT, "sqlite_get_sensorparams: unknown stationtype\n");
      sqsenspar.sensor_meas_no = -1;
      return(sqsenspar);
  }

  err = sqlite3_prepare( sqlitedb, query, -1, &qcomp, 0);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
      "sqlite_get_sensorparams: error: select sensor parameter info: "
      "err: %d : sqlite_errmsg: %s\n",
      err, sqlite3_errmsg(sqlitedb));
    sqsenspar.sensor_meas_no = -1;
    return(sqsenspar);
  }

  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {

    sqsenspar.sensor_meas_no = sqlite3_column_int(qcomp, 0);
    strncpy(sqsenspar.sensorname, 
      (char *)sqlite3_column_text(qcomp,1), TBUFF);
    strncpy(sqsenspar.par_name, 
      (char *)sqlite3_column_text(qcomp,2), TBUFF);
    sqsenspar.offset = sqlite3_column_double(qcomp,3);
    sqsenspar.gain   = sqlite3_column_double(qcomp,4);
    if ( stationtype == ONEWIRE ) {				  
      strncpy(sqsenspar.devicetyp, 
        (char *)sqlite3_column_text(qcomp,5), TBUFF);
      strncpy(sqsenspar.familycode, 
        (char *)sqlite3_column_text(qcomp,6), TBUFF);
      strncpy(sqsenspar.serialnum, 
        (char *)sqlite3_column_text(qcomp,7), TBUFF);
    } else {
      strncpy(sqsenspar.devicetyp, 
        "n.a.", TBUFF);
      strncpy(sqsenspar.familycode, 
        "n.a.", TBUFF);
      strncpy(sqsenspar.serialnum, 
        "n.a.", TBUFF);
    }
    rowcnt++;
  }
  
  syslog(LOG_DEBUG, "sqlite_get_sensor_params: sensor_meas_no : %d\n", 
                    sqsenspar.sensor_meas_no);
  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "sqlite_get_sensorparams: error: select parametername: "
            "err: %d : sqlite_errmsg: %s\n",
            err, sqlite3_errmsg(sqlitedb));
    return(sqsenspar);
  }
  if ( rowcnt == 0) {
    syslog( LOG_DEBUG, "sqlite_get_sensorparams: "
                       "no configuration data in database");
    sqsenspar.sensor_meas_no = -1;
    return(sqsenspar);
  }

  return(sqsenspar);
}

sensflag_t
sqlite_get_sensorflags( char *sensorname, char *flagname,
                         int stationtype)
{
  int err;
  int rowcnt = 0;
  char query[SBUFF+1] = "\0";
  sensflag_t sqsensflag;
  sqlite3 *sqlitedb;
  sqlite3_stmt *qcomp;

   
  switch(stationtype) {
    case ONEWIRE:
      syslog(LOG_DEBUG, "sqlite_get_sensorflags: no flags available"
                        " for stationtype ONEWIRE\n");
      sqsensflag.sensor_flag_no = -1;
      return(sqsensflag);
      break;
    case WMR9X8:
      syslog(LOG_DEBUG, "sqlite_get_sensorflags: stationtype is "
                        "WMR9X8\n");
      sqlitedb = wmr9x8db;
      snprintf(query, SBUFF,
        "SELECT sf.sensor_flag_no, sn.sensor_no, fn.flag_no, sn.sensor_name, "
        "fn.flag_name "
        "FROM sensorflags AS sf, sensornames AS sn, "
        "flagnames AS fn "
        "WHERE sf.flag_no = fn.flag_no "
        "AND sf.sensor_no = sn.sensor_no "
        "AND sn.sensor_name = '%s' "
        "AND fn.flag_name  = '%s' ",
        sensorname, flagname);
      syslog(LOG_DEBUG, "sqlite_get_sensorflags: query: %s\n",
                        query);
      break;
    case UMETER:
      syslog(LOG_DEBUG, "sqlite_get_sensorflags: stationtype is "
                        "UMETER\n");
      sqlitedb = umeterdb;
      snprintf(query, SBUFF,
        "SELECT sf.sensor_flag_no, sn.sensor_no, fn.flag_no, sn.sensor_name, "
        "fn.flag_name "
        "FROM sensorflags AS sf, sensornames AS sn, "
        "flagnames AS fn "
        "WHERE sf.flag_no = fn.flag_no "
        "AND sf.sensor_no = sn.sensor_no "
        "AND sn.sensor_name = '%s' "
        "AND fn.flag_name  = '%s' ",
        sensorname, flagname);
      syslog(LOG_DEBUG, "sqlite_get_sensorflags: query: %s\n",
                        query);
      break;
    default:
      syslog(LOG_ALERT, "sqlite_get_sensorparams: "
                        "unknown stationtype\n");
      sqsensflag.sensor_flag_no = -1;
      return(sqsensflag);
  }

  err = sqlite3_prepare( sqlitedb, query, -1, &qcomp, 0);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
      "sqlite_get_sensorflags: error: select sensor flag info: "
      "err: %d : sqlite_errmsg: %s\n",
      err, sqlite3_errmsg(sqlitedb));
    sqsensflag.sensor_flag_no = -1;
    return(sqsensflag);
  }

  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {

    sqsensflag.sensor_flag_no = sqlite3_column_int(qcomp, 0);
    sqsensflag.sensor_no = sqlite3_column_int(qcomp, 1);
    sqsensflag.flag_no = sqlite3_column_int(qcomp, 2);
    strncpy(sqsensflag.sensorname, 
      (char *)sqlite3_column_text(qcomp,3), TBUFF);
    strncpy(sqsensflag.flagname, 
      (char *)sqlite3_column_text(qcomp,4), TBUFF);

    strncpy(sqsensflag.devicetyp, 
      "n.a.", TBUFF);
    strncpy(sqsensflag.familycode, 
      "n.a.", TBUFF);
    strncpy(sqsensflag.serialnum, 
      "n.a.", TBUFF);
   
    rowcnt++;
  }
  
  syslog(LOG_DEBUG, "sqlite_get_sensorflags: sensor_flag_no : %d\n", 
                    sqsensflag.sensor_flag_no);
  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "sqlite_get_sensorflags: error: select flagname: "
            "err: %d : sqlite_errmsg: %s\n",
            err, sqlite3_errmsg(sqlitedb));
    return(sqsensflag);
  }

  if ( rowcnt == 0) {
    syslog( LOG_DEBUG, "sqlite_get_sensorflags: "
                       "no configuration data in database");
    sqsensflag.sensor_flag_no = -1;
    return(sqsensflag);
  }

  return(sqsensflag);
}

/*
  sqlite_datadbn 

  insert measured data values for use in 
    - 1-Wire
    - WS2000 
    - PCWSR 
  database

*/
int
sqlite_datadbn( long dataset_date, int sensor_meas_no, float meas_value,
  int stationtype )
{
  int          err;
  int          querylen = MAXQUERYLEN;
  char         query[MAXQUERYLEN];
  sqlite3      *sqlitedb;

  switch(stationtype) {
    case ONEWIRE:
      syslog(LOG_DEBUG, "sqlite_datadbn: stationtype is "
        "ONEWIRE\n");
      sqlitedb = onewiredb;
      break;
    case UMETER:
      syslog(LOG_DEBUG, "sqlite_datadbn: stationtype is "
        "UMETER\n");
      sqlitedb = umeterdb;
      break;
    case WMR9X8:
      syslog(LOG_DEBUG, "sqlite_datadbn: stationtype is "
        "WMR9X8\n");
      sqlitedb = wmr9x8db;
      break;
    case WS2000:
      syslog(LOG_DEBUG, "sqlite_datadbn: stationtype is "
        "WS2000\n");
      sqlitedb = ws2000db;
      break;
    default:
      syslog(LOG_ALERT, "sqlite_datadbn: unknown stationtype\n");
      return(1);
  }  

  /* insert data values */
  syslog(LOG_DEBUG, "sqlite_datadbn: dataset_date: %ld, "
                    "sensor_meas_no: %d, meas_val: %f", 
                    dataset_date, sensor_meas_no, meas_value);
  snprintf(query, querylen, 
    "INSERT INTO sensordata VALUES ( NULL, %lu, %d, %f)",
    dataset_date, sensor_meas_no, meas_value); 
  syslog(LOG_DEBUG, "sqlite_datadbn: query: %s", query);
  err = sqlite3_exec( sqlitedb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_DEBUG,
      "datadb: error: insert sensor data: err: %d", err);
  }

  /* insert date when last data has been updated */
  snprintf(query, querylen, 
    "UPDATE sensorupdate SET last_update = %lu WHERE sensor_meas_no = %d",
    dataset_date, sensor_meas_no); 
  syslog(LOG_DEBUG, "sqlite_datadbn: query: %s", query);
  err = sqlite3_exec( sqlitedb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_DEBUG, 
      "datadb: error: update sensor date: err: %d", err);
  }

  return(err);
}

/*
  sqlite_statdbn 

  insert measured status values for use in 
    - 1-Wire
    - WS2000 
    - PCWSR 
  database

*/
int
sqlite_statdbn( long statusset_date, int sensor_flag_no, 
                long unsigned int stat_value, int stationtype )
{
  int          err;
  int          querylen = MAXQUERYLEN;
  char         query[MAXQUERYLEN];
  sqlite3      *sqlitedb;

  switch(stationtype) {
    case ONEWIRE:
      syslog(LOG_DEBUG, "sqlite_statdbn: stationtype is "
        "ONEWIRE\n");
      sqlitedb = onewiredb;
      break;
    case UMETER:
      syslog(LOG_DEBUG, "sqlite_statdbn: stationtype is "
        "UMETER\n");
      sqlitedb = umeterdb;
      break;
    case WMR9X8:
      syslog(LOG_DEBUG, "sqlite_statdbn: stationtype is "
        "WMR9X8\n");
      sqlitedb = wmr9x8db;
      break;
    default:
      syslog(LOG_ALERT, "sqlite_statdbn: unknown stationtype\n");
      return(1);
  }  

  /* insert status values */
  syslog(LOG_DEBUG, "sqlite_statdbn: dataset_date: %ld, "
                    "sensor_flag_no: %d, stat_val: %lu", 
                    statusset_date, sensor_flag_no, stat_value);
  snprintf(query, querylen, 
    "INSERT INTO statusdata VALUES ( NULL, %lu, %d, %lu)",
    statusset_date, sensor_flag_no, stat_value); 
  syslog(LOG_DEBUG, "sqlite_statdbn: query: %s", query);
  err = sqlite3_exec( sqlitedb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_DEBUG,
      "sqlite_statdbn: error: insert sensor data: err: %d", err);
  }

  err = sqlite3_exec( sqlitedb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_DEBUG, 
      "datadb: error: update sensor date: err: %d", err);
  }

  return(err);
}

